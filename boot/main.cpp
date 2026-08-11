#include "boot_context.h"
#include "kernel.h"
#include "file.h"
#include "elfheader.h"
#include "paging.h"
#include "memory_map.h"

constexpr UINT64 ONE_KB = 1 * 1024;
constexpr UINT64 ONE_MB = ONE_KB * 1024;
constexpr UINT64 ONE_GB = ONE_MB * 1024;
constexpr UINT64 PAGE_SIZE = 4096;

BootCtx *ctx = nullptr;
PML4E *pml4t = nullptr;

template <typename T>
EFI_STATUS LocateProtocol(const EFI_GUID &guid, T **output)
{
    return ctx->boot_services()->LocateProtocol(const_cast<EFI_GUID *>(&guid),
                                                nullptr,
                                                reinterpret_cast<void **>(output));
}

template <typename T>
EFI_STATUS HandleProtocol(EFI_HANDLE handle, const EFI_GUID &guid, T **output)
{
    return ctx->boot_services()->HandleProtocol(handle,
                                                const_cast<EFI_GUID *>(&guid),
                                                reinterpret_cast<void **>(output));
}

KernInfo read_kernel()
{
    KernInfo kernel_info;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = nullptr;
    EFI_GUID guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    if (EFI_ERROR(HandleProtocol<EFI_SIMPLE_FILE_SYSTEM_PROTOCOL>(ctx->loaded_image()->DeviceHandle,
                                                                  guid, &sfs)))
        ctx->halt(L"FAILED TO LOAD SFS PROTOCOL");


    EFI_FILE_HANDLE volHandle = nullptr;
    if (EFI_SUCCESS != sfs->OpenVolume(sfs, &volHandle))
        ctx->halt(L"FAILED TO OPEN VOLUME");

    EfiFile khandle = EfiFile(volHandle, L"\\Kernel.elf");

    ELF_HEADER *elf_header = nullptr;
    UINTN read_size = sizeof(ELF_HEADER);

    if (EFI_SUCCESS !=
        ctx->boot_services()->AllocatePool(EfiBootServicesData, sizeof(ELF_HEADER), (void **)&elf_header))
      ctx->halt(L"FAILED TO ALLOCATE MEM FOR ELF_HEADER");

    if (EFI_ERROR(khandle.read(read_size, (void *)elf_header)))
        ctx->halt(L"FAILED TO READ ELF HEADER");

    if (elf_header->ei_magic != ELF_MAGIC) {
        ctx->halt(L"ELF MAGIC NOT MATCHING");
    }

    UINTN phsize = elf_header->e_phnum * elf_header->e_phentsize;
    ELF_PROG_HEADER *pheader = nullptr;

    if (EFI_ERROR(khandle.seek(elf_header->e_phoff)))
        ctx->halt(L"FAILED TO SET POSITION");

    if (EFI_SUCCESS !=
        ctx->boot_services()->AllocatePool(EfiBootServicesData, phsize, (void **)&pheader))
      ctx->halt(L"FAILED TO ALLOCATE MEM FOR ELF_HEADER");

    if (EFI_ERROR(khandle.read(phsize, (void *)pheader)))
        ctx->halt(L"FAILED TO READ PROGRAM HEADER");

    kernel_info.min_addr = -1;
    kernel_info.max_addr = 0;

    for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
        if (pheader[i].p_type != 1) continue;
        if (pheader[i].p_paddr < kernel_info.min_addr)
            kernel_info.min_addr = pheader[i].p_vaddr;
        if (pheader[i].p_vaddr + pheader[i].p_memsz > kernel_info.max_addr)
            kernel_info.max_addr = pheader[i].p_vaddr + pheader[i].p_memsz;
    }

    kernel_info.kernel_size = kernel_info.max_addr - kernel_info.min_addr;
    kernel_info.kernel_pages = (kernel_info.kernel_size + 4095) >> 12;

    uint8_t *kernel_entry = nullptr;
    if (EFI_SUCCESS !=
        ctx->boot_services()->AllocatePages(AllocateAnyPages,EfiLoaderData, kernel_info.kernel_pages, (EFI_PHYSICAL_ADDRESS *)&kernel_entry))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR KERNEL");

    ctx->boot_services()->SetMem(kernel_entry, kernel_info.kernel_pages << 12, 0);

    uint8_t *itr = kernel_entry;
    for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
        if (pheader[i].p_type != 1)
            continue;

        UINTN offset = pheader[i].p_vaddr - kernel_info.min_addr;
        itr = kernel_entry + offset;

        if (EFI_ERROR(khandle.seek( pheader[i].p_offset)))
            ctx->halt(L"FAILED TO SET POSITION");

        auto phsize = pheader[i].p_filesz;
        if (EFI_ERROR(khandle.read(phsize, (void *)itr)))
            ctx->halt(L"FAILED TO READ PROGRAM HEADER");
    }

    return kernel_info;
}

EFI_STATUS
init_gop()
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = nullptr;

    if (EFI_ERROR(LocateProtocol<EFI_GRAPHICS_OUTPUT_PROTOCOL>(EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, &gop)))
        ctx->halt(L"Failed to locate GOP protocol");

    for (auto i = 0; i < gop->Mode->MaxMode; i++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = nullptr;
        UINTN size = 0;
        if (EFI_SUCCESS != gop->QueryMode(gop, i, &size, &info))
          ctx->halt(L"FAILED TO READ GOP MODE");

        if (info->HorizontalResolution == 800 &&
            info->VerticalResolution == 800) {
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

// Map the entire usable memory to the top of memory
EFI_STATUS
setup_paging(KernInfo kernel_info, void *kernel_entry, const UINTN total_mem)
{
    // Convert to nearest 1 GB addr
    const UINTN num_gb = (total_mem + (ONE_GB - 1)) / ONE_GB;

    if (pml4t == nullptr)
        ctx->halt(L"PML4 not setup");

    PDPE *pdpt = nullptr;
    if (EFI_ERROR(ctx->boot_services()->AllocatePages(AllocateAnyPages,EfiLoaderData,
                                               1, (EFI_PHYSICAL_ADDRESS *)&pdpt)))
      ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDPT");

    pml4t[256].PDPT = reinterpret_cast<uint64_t>(pdpt);
    pml4t[256].P = 1;
    pml4t[256].RW = 1;

    UINT64 paddr = 0x0;
    for (int i = 0; i < num_gb; i++) {
        pdpt[i].pdpe_1gb.P = 1;
        pdpt[i].pdpe_1gb.RW = 1;
        pdpt[i].pdpe_1gb.PS = 1;       // 1GB mapping
        pdpt[i].pdpe_1gb.PDT = paddr;
        paddr += ONE_GB;
    }

    // Map the kernel to compiled addr
    UINT64 image_base = reinterpret_cast<UINT64>(kernel_entry);
    const UINTN num_pte = kernel_info.kernel_pages;
    const UINTN num_pt   = num_pte >> 12;
    const UINTN num_pdpt = (num_pt >> 12) + 1;
    const UINTN num_pdt  = (num_pdpt >> 12) + 1;
    const UINT16 pml4_idx = (image_base >> 12) & 0x1ff;
    const UINT16 pdpt_idx = (image_base >> 21) & 0x1ff;
    const UINT16 pdt_idx  = (image_base >> 30) & 0x1ff;
    const UINT16 pt_idx   = (image_base >> 39) & 0x1ff;

    for (int i = 0; i < num_pt; i++) {
    }

    return EFI_SUCCESS;
}

void
identity_map_image()
{
    if (ctx->loaded_image() == nullptr)
        ctx->halt(L"NO LOADED IMAGE PROTOCOL");

    UINT64 image_base = reinterpret_cast<UINT64>(ctx->loaded_image()->ImageBase);
    UINT64 image_size = reinterpret_cast<UINT64>(ctx->loaded_image()->ImageSize);

    // To identity map the image, we will use 2MB identity paging
    // Round up to nearest 2 MB
    constexpr UINT64 TWO_MB = 2 * ONE_MB;
    const UINT16 pml4_idx = (image_base >> 12) & 0x1ff;
    const UINT16 pdpt_idx = (image_base >> 21) & 0x1ff;
    const UINT16 pdt_idx  = (image_base >> 30) & 0x1ff;
    const UINT16 pt_idx   = (image_base >> 39) & 0x1ff;
    const UINT16 image_size_2mb = (image_size + (TWO_MB - 1)) / TWO_MB;

    if (EFI_ERROR(ctx->boot_services()->AllocatePages(AllocateAnyPages,EfiLoaderData,
                                                      1, (EFI_PHYSICAL_ADDRESS *)&pml4t)))
        ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PML4");

    PDPE *pdpt = reinterpret_cast<PDPE *>(&pml4t[pml4_idx]);

    if (EFI_ERROR(ctx->boot_services()->AllocatePages(AllocateAnyPages,EfiLoaderData,
                                                      1, (EFI_PHYSICAL_ADDRESS *)&pdpt)))
        ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDPT");
    else {
        pml4t[pml4_idx].PDPT = reinterpret_cast<UINT64>(pdpt);
        pml4t[pml4_idx].P = 1;
        pml4t[pml4_idx].RW = 1;
    }

    PDE *pdt = reinterpret_cast<PDE *>(&pdpt[pdpt_idx]);

    if (EFI_ERROR(ctx->boot_services()->AllocatePages(AllocateAnyPages,EfiLoaderData,
                                                      image_size_2mb, (EFI_PHYSICAL_ADDRESS *)&pdt)))
        ctx->halt(L"FAILED TO ALLOCATE PAGES FOR PDT");
    else {
        pdpt[pdpt_idx].pdpe.PDT = reinterpret_cast<UINT64>(pdt);
        pdpt[pdpt_idx].pdpe.P = 1;
        pdpt[pdpt_idx].pdpe.RW = 1;
        pdpt[pdpt_idx].pdpe.PS = 1;
    }

    for (int i = 0; i < image_size_2mb; i++) {
        pdt[pdt_idx].pde_2mb.PT = image_base & TWO_MB;
        pdt[pdt_idx].pde_2mb.P  = 1;
        pdt[pdt_idx].pde_2mb.RW = 1;
        pdt[pdt_idx].pde_2mb.PS = 1;
        image_base += TWO_MB;
    }
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    auto boot_ctx = BootCtx(SystemTable, ImageHandle);
    ctx = &boot_ctx;

    EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
    EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    if (EFI_ERROR(HandleProtocol<EFI_LOADED_IMAGE_PROTOCOL>(ImageHandle,
                                                            guid,
                                                            &loadedImage)))
        ctx->halt(L"FAILED TO LOAD LOADED IMAGE PROTOCOL");

    ctx->set_loaded_image(loadedImage);
    ctx->clrscr();

    if (EFI_ERROR(ctx->boot_services()->SetWatchdogTimer(0, 0, 0, nullptr)))
      ctx->halt(L"FAILED TO SET WATCHDOG TIMER");

    init_gop();
    identity_map_image();

    auto kernel_info = read_kernel();

    MemoryMap mem_map;

    ctx->boot_services()->GetMemoryMap(&mem_map.size, nullptr, &mem_map.key, &mem_map.dsize,
                                &mem_map.dver);

    mem_map.size += PAGE_SIZE;
    mem_map.num_desc = mem_map.size / mem_map.dsize;

    if (EFI_ERROR(ctx->boot_services()->AllocatePool(EfiBootServicesData, mem_map.size, (void **)&mem_map.mm)))
        ctx->halt(L"FAILED TO ALLOCATE MEM FOR MEMORY MAP");

    if (EFI_ERROR(ctx->boot_services()->GetMemoryMap(&mem_map.size, (EFI_MEMORY_DESCRIPTOR *)mem_map.mm,
                                              &mem_map.key, &mem_map.dsize,
                                              &mem_map.dver)))
      ctx->halt(L"FAILED TO GET MEMORY MAP");


    for (int i = 0; i < mem_map.num_desc; i++) {
      EFI_MEMORY_DESCRIPTOR *pdesc =
          reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(mem_map.mm + (i * mem_map.dsize));

      if (pdesc->Type == EfiConventionalMemory ||
          pdesc->Type == EfiLoaderData || pdesc->Type == EfiLoaderCode ||
          pdesc->Type == EfiBootServicesData || pdesc->Type == EfiBootServicesCode)
          mem_map.total_mem += pdesc->NumberOfPages << 12;

      if (pdesc->PhysicalStart < mem_map.min_paddr)
          mem_map.min_paddr = pdesc->PhysicalStart;

      if ((pdesc->PhysicalStart + (pdesc->NumberOfPages << 12)) > mem_map.max_paddr)
          mem_map.max_paddr = (pdesc->PhysicalStart + (pdesc->NumberOfPages << 12));

      if (pdesc->VirtualStart < mem_map.min_vaddr)
          mem_map.min_vaddr = pdesc->VirtualStart;

      if ((pdesc->VirtualStart + (pdesc->NumberOfPages << 12)) > mem_map.max_vaddr)
          mem_map.max_vaddr = (pdesc->VirtualStart + (pdesc->NumberOfPages << 12));
    }

    uint8_t *pageTables = nullptr;
    if (EFI_ERROR(setup_paging(kernel_info, &pageTables, mem_map.total_mem)))
        ctx->halt(L"FAILED TO SETUP PAGES");


    if (EFI_ERROR(ctx->boot_services()->ExitBootServices(ImageHandle, mem_map.key)))
      ctx->halt(L"FAILED TO EXIT BOOT SERVICES");

    ctx->halt(L"SUCCESS");
    return EFI_SUCCESS;
}
