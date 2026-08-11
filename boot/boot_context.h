#pragma once
#include "efi.h"

class BootCtx {
public:
  BootCtx(EFI_SYSTEM_TABLE *pSystemTable, EFI_HANDLE handle)
      : m_st(pSystemTable), m_handle(handle) {
        m_bs = m_st->BootServices;
  }

  EFI_SYSTEM_TABLE *system_table() { return m_st; }
  EFI_BOOT_SERVICES *boot_services() { return m_bs; }
  EFI_HANDLE image_handle() { return m_handle; }
  EFI_LOADED_IMAGE_PROTOCOL *loaded_image() { return m_loadedImage;}
  void clrscr() { m_st->ConOut->ClearScreen(m_st->ConOut); }

  void set_loaded_image(EFI_LOADED_IMAGE_PROTOCOL *loaded_image) {
      m_loadedImage = loaded_image;
  }

  void print(const CHAR16 *string)
  {
      m_st->ConOut->OutputString(m_st->ConOut,
                                 const_cast<CHAR16 *>(string));
  }

  void halt(const CHAR16 *string) {
      if (string)
          print(string);
      m_st->BootServices->Stall(0xFFFFFFFF);
  }

  void print_hex(uint64_t num)
  {
      print(L"0x");

      if (num == 0) {
          print(L"0\n");
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
      print(L"\n");
  }

private:
    EFI_SYSTEM_TABLE *m_st;
    EFI_BOOT_SERVICES *m_bs;
    EFI_HANDLE m_handle;
    EFI_LOADED_IMAGE_PROTOCOL *m_loadedImage;
};
