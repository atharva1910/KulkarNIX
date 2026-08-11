#pragma once
#include "efi.h"

class EfiFile {
public:
    EfiFile(EFI_FILE_HANDLE vol_handle, const CHAR16 *file_name)
        : m_volHandle(vol_handle) {
        m_volHandle->Open(m_volHandle, &m_fileHandle,
                          const_cast<CHAR16 *>(file_name), EFI_FILE_MODE_READ,
                          EFI_FILE_READ_ONLY);
    }

    ~EfiFile() {
        if (m_fileHandle) {
            m_volHandle->Close(m_fileHandle);
        }
    }

    EFI_STATUS seek(UINTN offset) {
        return m_fileHandle->SetPosition(m_fileHandle, offset);
    }

    EFI_STATUS read(UINTN read_size, void *buffer) {
        return m_fileHandle->Read(m_fileHandle, &read_size, buffer);
    }

private:
    EFI_FILE_HANDLE m_volHandle;
    EFI_FILE_HANDLE m_fileHandle;
};
