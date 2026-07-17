#include "efi.h"
#include "efiapi.h"
#include "efidef.h"
#include "efierr.h"
#include "efiprot.h"

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

    void *kaddr = (void *)0x100000;
    UINTN read_size = 4096;
    if (EFI_SUCCESS != fileHandle->Read(fileHandle, &read_size, kaddr))
        HALT(L"FAILED TO READ ELF HEADER");

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
