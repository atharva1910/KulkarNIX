#include "efi.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // 1. Clear the firmware terminal screen
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

    // 2. Print UTF-16 wide string string (indicated by L"...")
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello Natively from Windows UEFI!\r\n");

    // Hang indefinitely so you can see the text
    // All structs like EFI_SYSTEM_TABLE are fully defined for you
    while(1);
    return EFI_SUCCESS;
}
