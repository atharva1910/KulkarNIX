#include <stdint.h>
#include "boot_context.h"
#include "kernel.h"
#include "memory_map.h"
#include "KulkarNIX.h"
#include "PagingManager.h"
#include "KernelArgs.h"

constexpr uint64_t ONE_KB = 1 * 1024;
constexpr uint64_t ONE_MB = ONE_KB * 1024;
constexpr uint64_t ONE_GB = ONE_MB * 1024;

BootCtx *ctx = nullptr;

void *
page_allocator(uint64_t num_pages)
{
    void *ret = nullptr;
    if (EFI_SUCCESS == ctx->boot_services()->AllocatePages(
                           AllocateAnyPages, EfiLoaderData, num_pages,
                           (EFI_PHYSICAL_ADDRESS *)&ret)) {
        ctx->boot_services()->SetMem(ret, PAGE_SIZE * num_pages, 0);
    }
    return ret;
}

EFI_STATUS
init_gop() {
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = nullptr;
  const EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  if (EFI_ERROR(LocateProtocol<EFI_GRAPHICS_OUTPUT_PROTOCOL>(ctx, guid, &gop)))
    ctx->halt(L"Failed to locate GOP protocol");

  for (auto i = 0; i < gop->Mode->MaxMode; i++) {
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = nullptr;
    UINTN size = 0;
    if (EFI_SUCCESS != gop->QueryMode(gop, i, &size, &info))
      ctx->halt(L"FAILED TO READ GOP MODE");

    if (info->HorizontalResolution == 800 && info->VerticalResolution == 800) {
      if (EFI_ERROR(gop->SetMode(gop, i)))
        ctx->halt(L"FAILED TO SET GOP MODE");
    }
  }

  return EFI_SUCCESS;
}

bool is_1GB_map_supported(const UINTN total_mem) {
  // check cpuid here
  return true;
}

void*
setup_kernel_args(const MemoryMap &mm, const Kernel &kernel)
{
    uint64_t total_size = mm.m_size + sizeof(KernelArgs);
    uint64_t pages = CEILING<uint64_t>(total_size, PAGE_SIZE);
    auto kernel_args = reinterpret_cast<KernelArgs *>(page_allocator(pages));
    if (kernel_args == nullptr) {
        ctx->halt(L"FAILED TO SETUP KERNEL ARGS");
    }

    kernel_args->k_info.kernelPages = kernel.m_kernelPages;
    kernel_args->k_info.kernelSize = kernel.m_kernelSize;
    kernel_args->k_info.minAddr = kernel.m_minAddr;

    kernel_args->mm_info.total_memory = mm.m_total_mem;
    kernel_args->mm_info.dsize = mm.m_dsize;
    kernel_args->mm_info.size = mm.m_size;
    kernel_args->mm_info.num_desc = mm.m_num_desc;
    kernel_args->mm_info.mm = reinterpret_cast<uint8_t *>(PA2VA<uint8_t*>(mm.mm));

    const char * str = "KNIXARG";
    for(int i = 0; i < sizeof(kernel_args->magic) - 1; i++)
        kernel_args->magic[i] = str[i];
    return kernel_args;
}

EFI_STATUS
setup_paging(const PagingManager<PAGE_ALLOCATOR> &pagingMgr,
             const Kernel &kernel,
             const UINTN total_mem)
{
  const UINTN num_gb = (total_mem + (ONE_GB - 1)) / ONE_GB;

  uint64_t paddr = 0x0;
  uint64_t vaddr = HIGHER_MEMORY_VADDR;

  for (int i = 0; i < num_gb; i++) {
    pagingMgr.map_page_1gb(paddr, vaddr);
    paddr += ONE_GB;
    vaddr += ONE_GB;
  }

  const UINTN num_pte = kernel.m_kernelPages;
  paddr = reinterpret_cast<uint64_t>(kernel.m_kernelPaddr);
  vaddr = KERNEL_START_VADDR;
  for (int i = 0; i < num_pte; i++) {
      pagingMgr.map_page(paddr, vaddr);
      paddr += PAGE_SIZE;
      vaddr += PAGE_SIZE;
  }

  return EFI_SUCCESS;
}

void
identity_map_image(const PagingManager<PAGE_ALLOCATOR> &pagingMgr)
{
  if (ctx->loaded_image() == nullptr)
    ctx->halt(L"NO LOADED IMAGE PROTOCOL");

  UINT64 image_base = reinterpret_cast<UINT64>(ctx->loaded_image()->ImageBase);
  UINT64 image_size = reinterpret_cast<UINT64>(ctx->loaded_image()->ImageSize);

  constexpr UINT64 TWO_MB = 2 * ONE_MB;
  const UINT16 image_size_2mb = (image_size + (TWO_MB - 1)) / TWO_MB;
  image_base = image_base & ~(TWO_MB - 1);

  for (int i = 0; i < image_size_2mb; i++) {
    auto addr = image_base + (i * TWO_MB);
      ctx->print(L"Mapping TWO MB Pages: ");
      ctx->print_hex(addr);
      if (!pagingMgr.map_page_2mb(addr, addr)) {
          ctx->halt(L"FAILED TO MAP 2MB PAGE");
      }
  }
}



EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  auto boot_ctx = BootCtx(SystemTable, ImageHandle);
  ctx = &boot_ctx;
  ctx->clrscr();

  EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
  EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  if (EFI_ERROR(HandleProtocol<EFI_LOADED_IMAGE_PROTOCOL>(ctx, ImageHandle,
                                                          guid, &loadedImage)))
      ctx->halt(L"FAILED TO LOAD LOADED IMAGE PROTOCOL");

  ctx->set_loaded_image(loadedImage);

  if (EFI_ERROR(ctx->boot_services()->SetWatchdogTimer(0, 0, 0, nullptr)))
      ctx->halt(L"FAILED TO SET WATCHDOG TIMER");

  auto pagingMgr = PagingManager<PAGE_ALLOCATOR>(page_allocator);

  init_gop();
  identity_map_image(pagingMgr);

  Kernel kernel(ctx);
  MemoryMap mem_map(ctx);

  auto kernel_args = setup_kernel_args(mem_map, kernel);
  if (kernel_args == nullptr) {
    ctx->halt(L"FAILED TO SETUP KERNEL ARGS");
  } else {
      ctx->print(L"Kernel args: ");
      ctx->print_hex(reinterpret_cast<uint64_t>(kernel_args));
  }

  if (EFI_ERROR(setup_paging(pagingMgr, kernel, mem_map.m_total_mem)))
    ctx->halt(L"FAILED TO SETUP PAGES");

  ctx->print(L"Jumping to kernel: ");
  ctx->print_hex(kernel.m_kernelEntry);

  mem_map.refresh();

  if (EFI_ERROR(
          ctx->boot_services()->ExitBootServices(ImageHandle, mem_map.m_key)))
    ctx->halt(L"FAILED TO EXIT BOOT SERVICES");

  asm volatile("mov %[pml4], %%cr3\n\t"
               "mov %[args], %%rdi\n\t"
               "jmp *%[entry]\n\t"
               :
               : [pml4] "r"(pagingMgr.get_base()),
                 [entry] "r"(kernel.m_kernelEntry),
                 [args] "r" (reinterpret_cast<uint64_t>(kernel_args))
               : "memory", "rdi");
  return EFI_SUCCESS;
}
