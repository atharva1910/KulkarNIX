#include "efi.h"
#include "efiapi.h"
#include "efidef.h"
#include "KulkarNIX.h"
#include "elfheader.h"
#include <stdint.h>

EFI_SYSTEM_TABLE *pSystemTable = NULL;
EFI_BOOT_SERVICES *pBootServices = NULL;
EFI_HANDLE handle;
#define CLRSCR pSystemTable->ConOut->ClearScreen(pSystemTable->ConOut);
#define PRINT(X) pSystemTable->ConOut->OutputString(pSystemTable->ConOut, X);
#define HALT(X)                                                     \
    {                                                               \
        if (X)                                                      \
            PRINT(X)                                                \
        pSystemTable->BootServices->Stall(0xFFFFFFFFFF);            \
    }

void PrintNumber(uint64_t num)
{
    //PRINT(L"0");
    //PRINT(L"x");
    if (num == 0) {
        PRINT(L"0 ");
        return;
    }

    uint16_t numstr[16];
    int i = 15;
    numstr[i--] = '\0';

    while (num) {
        uint8_t temp = num % 10;
        num /= 10;
        numstr[i--] = temp + '0';
    }

    PRINT(&numstr[++i]);
    PRINT(L" ");
}

EFI_STATUS
ReadKernel()
{
    EFI_LOADED_IMAGE_PROTOCOL *loadedImage = NULL;
    EFI_GUID loadedImageGuid = (EFI_GUID)EFI_LOADED_IMAGE_PROTOCOL_GUID;
    if (EFI_SUCCESS != pBootServices->HandleProtocol(handle, &loadedImageGuid,
                                                     (void *)&loadedImage))
        HALT(L"FAILED TO LOAD LOADED IMAGE PROTOCOL");

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = NULL;
    EFI_GUID sfsGUID = (EFI_GUID)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    if (EFI_SUCCESS != pBootServices->HandleProtocol(loadedImage->DeviceHandle, &sfsGUID,
                                                     (void *)&sfs))
        HALT(L"FAILED TO LOAD SFS PROTOCOL");


    EFI_FILE_HANDLE volHandle = NULL;
    if (EFI_SUCCESS != sfs->OpenVolume(sfs, &volHandle))
        HALT(L"FAILED TO OPEN VOLUME");

    EFI_FILE_HANDLE fileHandle = NULL;
    if (EFI_SUCCESS !=
        volHandle->Open(volHandle, &fileHandle, L"\\Kernel.bin",EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY))
        HALT(L"FAILED TO OPEN FILE HANDLE");

    ELF_HEADER *kaddr = (ELF_HEADER *)KERNEL_START_PADDR;
    UINTN read_size = sizeof(ELF_HEADER);
    if (EFI_SUCCESS != fileHandle->Read(fileHandle, &read_size, (void *)kaddr))
        HALT(L"FAILED TO READ ELF HEADER");

    if (kaddr->ei_magic != ELF_MAGIC) {
        PrintNumber(kaddr->ei_magic);
        HALT(L"ELF MAGIC NOT MATCHING");
    }
        PrintNumber(kaddr->ei_magic);
    HALT(L"READ ELF HEADER");
    return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    pSystemTable = SystemTable;
    pBootServices = pSystemTable->BootServices;
    handle = ImageHandle;

    CLRSCR;
    if (EFI_SUCCESS == ReadKernel())
        HALT(L"SUCCESS");
    return EFI_SUCCESS;
}
