#pragma once
#include "boot_context.h"
#include "efi.h"
#include "file.h"
#include "elfheader.h"

class Kernel {
public:
    BootCtx *ctx = nullptr;
    uintptr_t m_kernelEntry;
    UINTN m_kernelPages;
    UINTN m_minAddr;
    UINTN m_maxAddr;
    UINTN m_kernelSize;


    Kernel(BootCtx *c) {
        ctx = c;
        read_kernel();
    }

    void read_kernel() {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = nullptr;
        EFI_GUID guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
        if (EFI_ERROR(HandleProtocol<EFI_SIMPLE_FILE_SYSTEM_PROTOCOL>(ctx, ctx->loaded_image()->DeviceHandle,
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

        m_kernelEntry = elf_header->e_entry;

        UINTN phsize = elf_header->e_phnum * elf_header->e_phentsize;
        ELF_PROG_HEADER *pheader = nullptr;

        if (EFI_ERROR(khandle.seek(elf_header->e_phoff)))
            ctx->halt(L"FAILED TO SET POSITION");

        if (EFI_SUCCESS !=
            ctx->boot_services()->AllocatePool(EfiBootServicesData, phsize, (void **)&pheader))
            ctx->halt(L"FAILED TO ALLOCATE MEM FOR ELF_HEADER");

        if (EFI_ERROR(khandle.read(phsize, (void *)pheader)))
            ctx->halt(L"FAILED TO READ PROGRAM HEADER");

        m_minAddr = -1;
        m_maxAddr = 0;

        for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
            if (pheader[i].p_type != 1) continue;
            if (pheader[i].p_paddr < m_minAddr)
                m_minAddr = pheader[i].p_vaddr;
            if (pheader[i].p_vaddr + pheader[i].p_memsz > m_maxAddr)
                m_maxAddr = pheader[i].p_vaddr + pheader[i].p_memsz;
        }

        m_kernelSize = m_maxAddr - m_minAddr;
        m_kernelPages = (m_kernelSize + 4095) >> 12;

        uint8_t *kernel_entry = nullptr;
        if (EFI_SUCCESS !=
            ctx->boot_services()->AllocatePages(AllocateAnyPages,EfiLoaderData, m_kernelPages, (EFI_PHYSICAL_ADDRESS *)&kernel_entry))
            ctx->halt(L"FAILED TO ALLOCATE PAGES FOR KERNEL");

        ctx->boot_services()->SetMem(kernel_entry, m_kernelPages << 12, 0);

        uint8_t *itr = kernel_entry;
        for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
            if (pheader[i].p_type != 1)
                continue;

            UINTN offset = pheader[i].p_vaddr - m_minAddr;
            itr = kernel_entry + offset;

            if (EFI_ERROR(khandle.seek( pheader[i].p_offset)))
                ctx->halt(L"FAILED TO SET POSITION");

            auto phsize = pheader[i].p_filesz;
            if (EFI_ERROR(khandle.read(phsize, (void *)itr)))
                ctx->halt(L"FAILED TO READ PROGRAM HEADER");
        }
    }
};
