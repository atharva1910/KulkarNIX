#include "efi.h"
#include "efiapi.h"
#include "efidef.h"
#include "efierr.h"
#include "efiprot.h"

EFI_SYSTEM_TABLE *pSystemTable = NULL;
EFI_BOOT_SERVICES *pBootServices = NULL;
EFI_HANDLE handle;
#define PRINT(X) pSystemTable->ConOut->OutputString(pSystemTable->ConOut, X);
#define HALT(X)                                                     \
    {                                                               \
        if (X)                                                      \
            PRINT(X)                                                \
                pSystemTable->BootServices->Stall(0xFFFFFFFFFF);    \
    }


EFI_STATUS
ReadKernel()
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_HANDLE_PROTOCOL *loadedImage = NULL;
    EFI_GUID loadedImageGuid = (EFI_GUID)EFI_LOADED_IMAGE_PROTOCOL_GUID;
    status = pBootServices->HandleProtocol(handle, &loadedImageGuid,
                                        (void *)&loadedImage);
    if (status != EFI_SUCCESS) {
        HALT(L"FAILED TO LOAD LOADED IMAGE PROTOCOL");
    } else {
        HALT(L" LOADED IMAGE PROTOCOL Loaded");
    }

    return status;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    pSystemTable = SystemTable;
    pBootServices = pSystemTable->BootServices;
    handle = ImageHandle;

    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    if (EFI_SUCCESS == ReadKernel())
        HALT(L"SUCCESS");
    return EFI_SUCCESS;
}
