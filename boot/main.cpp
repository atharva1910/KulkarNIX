#include <stdint.h>
#include "boot_context.h"
#include "kernel.h"
#include "memory_map.h"
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
setup_paging(const Kernel &kernel, const UINTN total_mem) {
  if (pml4t == nullptr)
    ctx->halt(L"PML4 not setup");

  const UINTN num_gb = (total_mem + (ONE_GB - 1)) / ONE_GB;

  PDPTE *pdpt = nullptr;
  if (EFI_ERROR(ctx->boot_services()->AllocatePages(
          AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pdpt)))
    ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDPT");

  pml4t[256].PDPT = reinterpret_cast<uint64_t>(pdpt) >> 12;
  pml4t[256].P = 1;
  pml4t[256].RW = 1;

  UINT64 paddr = 0x0;
  for (int i = 0; i < num_gb; i++) {
    pdpt[i].pdpe_1gb.P = 1;
    pdpt[i].pdpe_1gb.RW = 1;
    pdpt[i].pdpe_1gb.PS = 1; // 1GB mapping
    pdpt[i].pdpe_1gb.PDT = paddr >> 30;
    paddr += ONE_GB;
  }

  // Map the kernel to compiled addr
  UINT64 image_base = reinterpret_cast<UINT64>(kernel.m_kernelEntry);
  const UINTN num_pte = kernel.m_kernelPages;
  const UINTN num_pt = (num_pte >> 12) + 1;
  const UINTN num_pdt = (num_pt >> 12) + 1;
  const UINTN num_pdpt = (num_pdt >> 12) + 1;

  ctx->print(L"Image base: ");
  ctx->print_hex(image_base);

  // Todo: Make sure pte, pt, pdpt are 1
  const UINT16 pt_idx = (image_base >> 12) & 0x1ff;
  const UINT16 pdt_idx = (image_base >> 21) & 0x1ff;
  const UINT16 pdpt_idx = (image_base >> 30) & 0x1ff;
  const UINT16 pml4_idx = (image_base >> 39) & 0x1ff;

  pdpt = nullptr;
  if (pml4t[pml4_idx].P == 0) {
    if (EFI_ERROR(ctx->boot_services()->AllocatePages(
            AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pdpt)))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDPT");
    ctx->boot_services()->SetMem(pdpt, PAGE_SIZE, 0);
    pml4t[pml4_idx].PDPT = reinterpret_cast<uint64_t>(pdpt) >> 12;
    pml4t[pml4_idx].P = 1;
    pml4t[pml4_idx].RW = 1;
  } else {
    pdpt = reinterpret_cast<PDPTE *>(pml4t[pml4_idx].PDPT << 12);
  }

  PDTE *pdt = nullptr;
  if (pdpt[pdpt_idx].pdpe.P == 0) {
    if (EFI_ERROR(ctx->boot_services()->AllocatePages(
            AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pdt)))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDT");
    ctx->boot_services()->SetMem(pdt, PAGE_SIZE, 0);
    pdpt[pdpt_idx].pdpe.PDT = reinterpret_cast<uint64_t>(pdt) >> 12;
    pdpt[pdpt_idx].pdpe.P = 1;
    pdpt[pdpt_idx].pdpe.RW = 1;
  } else {
    pdt = reinterpret_cast<PDTE *>(pdpt[pdpt_idx].pdpe.PDT << 12);
  }

  PTE *pte = nullptr;
  if (pdt[pdt_idx].pde.P == 0) {
    if (EFI_ERROR(ctx->boot_services()->AllocatePages(
            AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pte)))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PT");
    ctx->boot_services()->SetMem(pte, PAGE_SIZE, 0);
    pdt[pdt_idx].pde.PT = reinterpret_cast<uint64_t>(pte) >> 12;
    pdt[pdt_idx].pde.P = 1;
    pdt[pdt_idx].pde.RW = 1;
  } else {
    pte = reinterpret_cast<PTE *>(pdt[pdt_idx].pde.PT << 12);
  }

  uint64_t addr = kernel.m_kernelPaddr;
  for (int i = 0; i < num_pte; i++) {
    if (pt_idx + i >= PAGE_TABLE_NUM_ENTRIES)
      ctx->halt(L"OUT OF RANGE");
    pte[pt_idx + i].P = 1;
    pte[pt_idx + i].RW = 1;
    pte[pt_idx + i].PAGE = addr >> 12;
    addr += PAGE_SIZE;
  }

  return EFI_SUCCESS;
}

void identity_map_image() {
  if (ctx->loaded_image() == nullptr)
    ctx->halt(L"NO LOADED IMAGE PROTOCOL");

  UINT64 image_base = reinterpret_cast<UINT64>(ctx->loaded_image()->ImageBase);
  UINT64 image_size = reinterpret_cast<UINT64>(ctx->loaded_image()->ImageSize);

  // To identity map the image, we will use 2MB identity paging
  // Round up to nearest 2 MB
  constexpr UINT64 TWO_MB = 2 * ONE_MB;
  const UINT16 pt_idx = (image_base >> 12) & 0x1ff;
  const UINT16 pdt_idx = (image_base >> 21) & 0x1ff;
  const UINT16 pdpt_idx = (image_base >> 30) & 0x1ff;
  const UINT16 pml4_idx = (image_base >> 39) & 0x1ff;
  const UINT16 image_size_2mb = (image_size + (TWO_MB - 1)) / TWO_MB;

  if (pml4t == nullptr) {
    if (EFI_ERROR(ctx->boot_services()->AllocatePages(
            AllocateAnyPages, EfiLoaderData, 1,
            (EFI_PHYSICAL_ADDRESS *)&pml4t)))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PML4");
    ctx->boot_services()->SetMem(pml4t, PAGE_SIZE, 0);
  }

  PDPTE *pdpt = nullptr;
  if (pml4t[pml4_idx].P == 0) {
    if (EFI_ERROR(ctx->boot_services()->AllocatePages(
            AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pdpt)))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDPT");
    ctx->boot_services()->SetMem(pdpt, PAGE_SIZE, 0);
    pml4t[pml4_idx].PDPT = reinterpret_cast<UINT64>(pdpt) >> 12;
    pml4t[pml4_idx].P = 1;
    pml4t[pml4_idx].RW = 1;
  } else {
    pdpt = reinterpret_cast<PDPTE *>(pml4t[pml4_idx].PDPT << 12);
  }

  PDTE *pdt = nullptr;
  if (pdpt[pdpt_idx].pdpe.P == 0) {
    if (EFI_ERROR(ctx->boot_services()->AllocatePages(
            AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pdt)))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDT");
    ctx->boot_services()->SetMem(pdt, PAGE_SIZE, 0);
    pdpt[pdpt_idx].pdpe.PDT = reinterpret_cast<UINT64>(pdt) >> 12;
    pdpt[pdpt_idx].pdpe.P = 1;
    pdpt[pdpt_idx].pdpe.RW = 1;
  } else {
    pdt = reinterpret_cast<PDTE *>(pdpt[pdpt_idx].pdpe.PDT << 12);
  }

  image_base = image_base & ~(TWO_MB - 1);
  for (int i = 0; i < image_size_2mb; i++) {
    if (pdt_idx + i >= PAGE_TABLE_NUM_ENTRIES)
      ctx->halt(L"OVERFLOW");
    pdt[pdt_idx + i].pde_2mb.PT = image_base >> 21;
    pdt[pdt_idx + i].pde_2mb.P = 1;
    pdt[pdt_idx + i].pde_2mb.RW = 1;
    pdt[pdt_idx + i].pde_2mb.PS = 1;
    image_base += TWO_MB;
  }
}

void *page_allocator(uint64_t num_pages) {
  return nullptr;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  auto boot_ctx = BootCtx(SystemTable, ImageHandle);

  ctx = &boot_ctx;

  auto x = PagingManager<PAGE_ALLOCATOR>(page_allocator);

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
  identity_map_image();

  Kernel kernel(ctx);
  MemoryMap mem_map(ctx);

  if (EFI_ERROR(setup_paging(kernel, mem_map.m_total_mem)))
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
