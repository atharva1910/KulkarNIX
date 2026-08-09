#include "efi.h"
#include "efiapi.h"
#include "efidef.h"
#include "efierr.h"
#include "efiprot.h"
#include "elfheader.h"
#include "paging.h"
#include "x86_64/efibind.h"

EFI_SYSTEM_TABLE *pSystemTable = NULL;
EFI_BOOT_SERVICES *pBootServices = NULL;
EFI_HANDLE handle;
PML4E *pml4t = nullptr;
constexpr UINT64 ONE_GB = 1 * 1024 * 1024 * 1024;

void clrscr() { pSystemTable->ConOut->ClearScreen(pSystemTable->ConOut); }
void print(const CHAR16 *string)
{
    pSystemTable->ConOut->OutputString(pSystemTable->ConOut,
                                       const_cast<CHAR16 *>(string));
}

void halt(const CHAR16 *string) {
    if (string)
        print(string);
    pSystemTable->BootServices->Stall(0xFFFFFFFF);
}

void print_hex(uint64_t num)
{
    print(L"0x");

    if (num == 0) {
        print(L"0 ");
        return;
    }

    // Lookup table for hexadecimal characters
    static constexpr CHAR16 hex_digits[] = L"0123456789abcdef";

    // 64-bit uint can have up to 16 hex digits + 1 null terminator
    CHAR16 numstr[17];
    int i = 16;
    numstr[i--] = L'\0';

    while (num > 0) {
        numstr[i--] = hex_digits[num & 0xF];
        num >>= 4;
    }

    print(&numstr[i + 1]);
    print(L" ");
}

EFI_STATUS
read_kernel_header(uint8_t **kernel_entry,
                   UINTN& kernel_pages,
                   UINTN  &min_addr,
                   UINTN  &max_addr,
                   UINTN &kernel_size)
{
    EFI_LOADED_IMAGE_PROTOCOL *loadedImage = NULL;
    EFI_GUID loadedImageGuid = (EFI_GUID)EFI_LOADED_IMAGE_PROTOCOL_GUID;
    if (EFI_SUCCESS != pBootServices->HandleProtocol(handle, &loadedImageGuid,
                                                     (void **)&loadedImage))
        halt(L"FAILED TO LOAD LOADED IMAGE PROTOCOL");

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = NULL;
    EFI_GUID sfsGUID = (EFI_GUID)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    if (EFI_SUCCESS != pBootServices->HandleProtocol(loadedImage->DeviceHandle, &sfsGUID,
                                                     (void **)&sfs))
        halt(L"FAILED TO LOAD SFS PROTOCOL");


    EFI_FILE_HANDLE volHandle = NULL;
    if (EFI_SUCCESS != sfs->OpenVolume(sfs, &volHandle))
        halt(L"FAILED TO OPEN VOLUME");

    EFI_FILE_HANDLE fileHandle = NULL;
    if (EFI_SUCCESS !=
        volHandle->Open(volHandle, &fileHandle,
                        const_cast<CHAR16 *>(L"\\Kernel.elf"),
                        EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY))
        halt(L"FAILED TO OPEN FILE HANDLE");


    ELF_HEADER *elf_header = NULL;
    UINTN read_size = sizeof(ELF_HEADER);

    if (EFI_SUCCESS !=
        pBootServices->AllocatePool(EfiBootServicesData, sizeof(ELF_HEADER), (void **)&elf_header))
      halt(L"FAILED TO ALLOCATE MEM FOR ELF_HEADER");

    if (EFI_SUCCESS != fileHandle->Read(fileHandle, &read_size, (void *)elf_header))
        halt(L"FAILED TO READ ELF HEADER");

    if (elf_header->ei_magic != ELF_MAGIC) {
        halt(L"ELF MAGIC NOT MATCHING");
    }

    UINTN phsize = elf_header->e_phnum * elf_header->e_phentsize;
    ELF_PROG_HEADER *pheader = NULL;

    if (EFI_SUCCESS != fileHandle->SetPosition(fileHandle, elf_header->e_phoff))
        halt(L"FAILED TO SET POSITION");

    if (EFI_SUCCESS !=
        pBootServices->AllocatePool(EfiBootServicesData, phsize, (void **)&pheader))
      halt(L"FAILED TO ALLOCATE MEM FOR ELF_HEADER");

    if (EFI_SUCCESS != fileHandle->Read(fileHandle, &phsize, (void *)pheader))
        halt(L"FAILED TO READ PROGRAM HEADER");

    min_addr = -1;
    max_addr = 0;

    for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
        if (pheader[i].p_type != 1) continue;
        if (pheader[i].p_paddr < min_addr)
            min_addr = pheader[i].p_vaddr;
        if (pheader[i].p_vaddr + pheader[i].p_memsz > max_addr)
            max_addr = pheader[i].p_vaddr + pheader[i].p_memsz;
    }

    kernel_size = max_addr - min_addr;
    kernel_pages = (kernel_size + 4095) >> 12;

    if (EFI_SUCCESS !=
        pBootServices->AllocatePages(AllocateAnyPages,EfiLoaderData, kernel_pages, (EFI_PHYSICAL_ADDRESS *)kernel_entry))
      halt(L"FAILED TO ALLOCATE PAGES FOR KERNEL");

    pBootServices->SetMem(*kernel_entry, kernel_pages << 12, 0);

    uint8_t *itr = *kernel_entry;
    for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
        if (pheader[i].p_type != 1)
            continue;

        UINTN offset = pheader[i].p_vaddr - min_addr;
        itr = *kernel_entry + offset;

        if (EFI_SUCCESS != fileHandle->SetPosition(fileHandle, pheader[i].p_offset))
            halt(L"FAILED TO SET POSITION");

        auto phsize = pheader[i].p_filesz;
        if (EFI_SUCCESS != fileHandle->Read(fileHandle, &phsize, (void *)itr))
            halt(L"FAILED TO READ PROGRAM HEADER");
    }

    return EFI_SUCCESS;
}

EFI_STATUS
init_gop()
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = nullptr;
    EFI_GUID gopGUID = (EFI_GUID)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    if (EFI_SUCCESS !=
        pBootServices->LocateProtocol(&gopGUID, NULL, (void **)&gop))
        halt(L"Failed to locate GOP protocol");

    for (auto i = 0; i < gop->Mode->MaxMode; i++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = nullptr;
        UINTN size = 0;
        if (EFI_SUCCESS != gop->QueryMode(gop, i, &size, &info))
          halt(L"FAILED TO READ GOP MODE");

        if (info->HorizontalResolution == 800 &&
            info->VerticalResolution == 800) {
            if (EFI_ERROR(gop->SetMode(gop, i)))
              halt(L"FAILED TO SET GOP MODE");
        }
    }

    return EFI_SUCCESS;
}

bool is_1GB_map_supported(const UINTN total_mem) {
  // check cpuid here
    return true;
}

EFI_STATUS
setup_paging(UINTN kernel_pages, uint8_t **pageTable, const UINTN total_mem)
{
    UINTN num_pt   = kernel_pages;
    UINTN num_pdpt = (num_pt >> 12) + 1;
    UINTN num_pdt  = (num_pdpt >> 12) + 1;
    UINTN num_pml4 = (num_pdt >> 12) + 1;

    // Convert to nearest 1 GB addr
    const UINTN num_gb = (total_mem + (ONE_GB - 1)) / ONE_GB;

    pml4t = nullptr;
    if (EFI_ERROR(pBootServices->AllocatePages(AllocateAnyPages,EfiLoaderData,
                                               1, (EFI_PHYSICAL_ADDRESS *)&pml4t)))
      halt(L"FAILED TO ALLOCATE PAGES FOR PML4");

    PDPE *pdpt = nullptr;
    if (EFI_ERROR(pBootServices->AllocatePages(AllocateAnyPages,EfiLoaderData,
                                               1, (EFI_PHYSICAL_ADDRESS *)&pdpt)))
      halt(L"FAILED TO ALLOCATE PAGES FOR PDPT");

    pml4t[256].pdpt = reinterpret_cast<uint64_t>(pdpt);
    pml4t[256].P = 1;
    pml4t[256].RW = 1;

    UINT64 paddr = 0x0;
    for (int i = 0; i < num_gb; i++) {
        pdpt[i].P = 1;
        pdpt[i].RW = 1;
        pdpt[i].PS = 1;
        pdpt[i].pdt = paddr;
        paddr += ONE_GB;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    pSystemTable = SystemTable;
    pBootServices = pSystemTable->BootServices;
    handle = ImageHandle;

    clrscr();

    uint8_t *kernel_entry = nullptr;
    UINTN kernel_pages = 0;
    UINTN kernel_min_addr = -1, kernel_max_addr = 0, kernel_size = 0;
    if (EFI_ERROR(read_kernel_header(&kernel_entry,
                                     kernel_pages,
                                     kernel_min_addr,
                                     kernel_max_addr,
                                     kernel_size)))
        halt(L"FAILED TO READ KERNEL");

    init_gop();

    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINTN mem_map_size = 0;
    UINT32 desc_version = 0;
    UINT8 *pmem_map = nullptr;

    pBootServices->GetMemoryMap(&mem_map_size, NULL, &map_key, &desc_size,
                                &desc_version);

    mem_map_size += 4096;

    if (EFI_ERROR(pBootServices->AllocatePool(EfiBootServicesData, mem_map_size, (void **)&pmem_map)))
        halt(L"FAILED TO ALLOCATE MEM FOR MEMORY MAP");

    if (EFI_ERROR(pBootServices->GetMemoryMap(&mem_map_size, (EFI_MEMORY_DESCRIPTOR *)pmem_map,
                                              &map_key, &desc_size,
                                              &desc_version)))
      halt(L"FAILED TO GET MEMORY MAP");

    UINT32 num_desc = mem_map_size / desc_size;
    UINTN min_paddr = -1, max_paddr = 0;
    UINTN min_vaddr = -1, max_vaddr = 0;
    UINT64 total_mem = 0;

    for (int i = 0; i < num_desc; i++) {
      EFI_MEMORY_DESCRIPTOR *pdesc =
          reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(pmem_map + (i * desc_size));

      if (pdesc->Type == EfiConventionalMemory ||
          pdesc->Type == EfiLoaderData || pdesc->Type == EfiLoaderCode ||
          pdesc->Type == EfiBootServicesData || pdesc->Type == EfiBootServicesCode)
      total_mem += pdesc->NumberOfPages << 12;

      if (pdesc->PhysicalStart < min_paddr)
          min_paddr = pdesc->PhysicalStart;

      if ((pdesc->PhysicalStart + (pdesc->NumberOfPages << 12)) > max_paddr)
          max_paddr = (pdesc->PhysicalStart + (pdesc->NumberOfPages << 12));

      if (pdesc->VirtualStart < min_vaddr)
          min_vaddr = pdesc->VirtualStart;

      if ((pdesc->VirtualStart + (pdesc->NumberOfPages << 12)) > max_vaddr)
          max_vaddr = (pdesc->VirtualStart + (pdesc->NumberOfPages << 12));
    }

    uint8_t *pageTables = nullptr;
    if (EFI_ERROR(setup_paging(kernel_pages, &pageTables, total_mem)))
        halt(L"FAILED TO SETUP PAGES");


    if (EFI_ERROR(pBootServices->ExitBootServices(ImageHandle, map_key)))
      halt(L"FAILED TO EXIT BOOT SERVICES");

    halt(L"SUCCESS");
    return EFI_SUCCESS;
}
