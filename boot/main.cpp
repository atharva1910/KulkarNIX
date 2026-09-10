#include <stdint.h>
#include "boot_context.h"
#include "kernel.h"
#include "memory_map.h"
#include "KulkarNIX.h"
#include "paging_mgr.hpp"

constexpr uint64_t ONE_KB = 1 * 1024;
constexpr uint64_t ONE_MB = ONE_KB * 1024;
constexpr uint64_t ONE_GB = ONE_MB * 1024;

BootCtx *ctx = nullptr;
PML4E *pml4t = nullptr;

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

EFI_STATUS
setup_paging(const PagingManager<PAGE_ALLOCATOR> &pagingMgr,const Kernel &kernel, const UINTN total_mem)
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
  paddr = kernel.m_kernelPaddr;
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

  // To identity map the image, we will use 2MB identity paging
  // Round up to nearest 2 MB
  constexpr UINT64 TWO_MB = 2 * ONE_MB;
  const UINT16 image_size_2mb = (image_size + (TWO_MB - 1)) / TWO_MB;
  image_base = image_base & ~(TWO_MB - 1);
  for (int i = 0; i < image_size_2mb; i++) {
      auto addr = image_base + (i * TWO_MB);
      if (!pagingMgr.map_page_2mb(addr, addr)) {
          ctx->halt(L"FAILED TO MAP 2MB PAGE");
      }
  }
}

void *page_allocator(uint64_t num_pages) {
  return nullptr;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  auto boot_ctx = BootCtx(SystemTable, ImageHandle);

  ctx = &boot_ctx;

  auto pagingMgr = PagingManager<PAGE_ALLOCATOR>(page_allocator);

  EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
  EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  if (EFI_ERROR(HandleProtocol<EFI_LOADED_IMAGE_PROTOCOL>(ctx, ImageHandle,
                                                          guid, &loadedImage)))
    ctx->halt(L"FAILED TO LOAD LOADED IMAGE PROTOCOL");

  ctx->set_loaded_image(loadedImage);
  ctx->clrscr();

  if (EFI_ERROR(ctx->boot_services()->SetWatchdogTimer(0, 0, 0, nullptr)))
    ctx->halt(L"FAILED TO SET WATCHDOG TIMER");

  init_gop();
  identity_map_image(pagingMgr);

  Kernel kernel(ctx);
  MemoryMap mem_map(ctx);

  if (EFI_ERROR(setup_paging(pagingMgr, kernel, mem_map.m_total_mem)))
    ctx->halt(L"FAILED TO SETUP PAGES");

  ctx->print(L"Jumping to kernel: ");
  ctx->print_hex(kernel.m_kernelEntry);
  mem_map.refresh();

  if (EFI_ERROR(
          ctx->boot_services()->ExitBootServices(ImageHandle, mem_map.m_key)))
    ctx->halt(L"FAILED TO EXIT BOOT SERVICES");

  asm volatile("mov %[pml4], %%rax\n\t"
               "mov %%rax, %%cr3\n\t"
               //"mov %[args], %%r13\n\t"
               "jmp *%[entry]\n\t"
               :
               : [pml4] "r"(pml4t), [entry] "r"(kernel.m_kernelEntry)
               //[args] "r" (kernel)
               : "memory", "rax");
  return EFI_SUCCESS;
}
