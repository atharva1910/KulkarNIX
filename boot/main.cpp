#include "efi.h"
#include "efiapi.h"
#include "efidef.h"
#include "elfheader.h"

EFI_SYSTEM_TABLE *pSystemTable = NULL;
EFI_BOOT_SERVICES *pBootServices = NULL;
EFI_HANDLE handle;
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
read_kernel()
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

    uint64_t min_addr = -1;
    uint64_t max_addr = 0;

    for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
        if (pheader[i].p_type != 1) continue;
        if (pheader[i].p_paddr < min_addr)
            min_addr = pheader[i].p_vaddr;
        if (pheader[i].p_vaddr + pheader[i].p_memsz > max_addr)
            max_addr = pheader[i].p_vaddr + pheader[i].p_memsz;
    }

    UINTN kernel_size = max_addr - min_addr;
    UINTN kernel_pages = (kernel_size + 4095) >> 12;

    uint8_t *kernel_entry = NULL;
    if (EFI_SUCCESS !=
        pBootServices->AllocatePages(AllocateAnyPages,EfiLoaderData, kernel_pages, (EFI_PHYSICAL_ADDRESS *)&kernel_entry))
      halt(L"FAILED TO ALLOCATE PAGES FOR KERNEL");

    pBootServices->SetMem(kernel_entry, kernel_pages << 12, 0);

    uint8_t *itr = kernel_entry;
    for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
        if (pheader[i].p_type != 1)
            continue;

        UINTN offset = pheader[i].p_vaddr - min_addr;
        itr = kernel_entry + offset;

        if (EFI_SUCCESS != fileHandle->SetPosition(fileHandle, pheader[i].p_offset))
            halt(L"FAILED TO SET POSITION");

        auto phsize = pheader[i].p_filesz;
        if (EFI_SUCCESS != fileHandle->Read(fileHandle, &phsize, (void *)itr))
            halt(L"FAILED TO READ PROGRAM HEADER");
    }

    return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    auto test = L"TEST";
    pSystemTable = SystemTable;
    pBootServices = pSystemTable->BootServices;
    handle = ImageHandle;
    clrscr();
    if (EFI_SUCCESS == read_kernel())
        halt(L"SUCCESS");
    return EFI_SUCCESS;
}
