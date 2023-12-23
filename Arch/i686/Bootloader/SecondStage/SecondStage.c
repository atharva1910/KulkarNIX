#include "SecondStage.h"

/*
 *  This file contains the main secondary boot loader and a bare bones ATA driver
 *  It reads the Kernel from the disk to location 0x10000 and jumps to the kernel entry point
 */

extern void SetupPagingAsm();
extern void LoadGDTAsm();
extern void SecondStageMain(uint32_t mmapAddr);

__asm__(
    ".section .text\n"
    ".global __start\n"
    ".type   __start, @function\n\n"
"__start:\n"
    /* Lets clear the interrupts just to be sure */
    "cli\n"

    /* The first stage should have pushed datasegment and mmap address in that order */
    "pop     %ax\n"
    "pop     %bx\n"
    "mov     %ax, %ds\n"
    "mov     %ax, %ss\n"
    "mov     %ax, %es\n"

    /* Lets setup out stack */
    "mov     $0x7BFF, %ax\n"
    "xor     %sp, %sp\n"
    "xor     %bp, %bp\n"
    "mov     %ax, %sp\n"
    "mov     %ax, %bp\n"

    /* Call the Second stage bootloader */
    "push    %ebx\n"
    "call    SecondStageMain\n"
    "hlt\n"
);

static inline void
PrintChar(char *address, char c, byte bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}

static inline void
PrintString(const char *string)
{
    char *vga_buffer = (char *)0xb8000;
    char c = 0;
    uint32_t pos = 0;
    while((c = string[pos++]) != '\0'){
        PrintChar(vga_buffer, c, 0x07);
        vga_buffer += 2;
    }
}

void AtaDiskWait()
{
    while((inb(0x1F7) & 0xC0) != 0x40);
}

void ReadSector(uint32_t sector)
{
    AtaDiskWait(); // wait BSY to 0 and RDY to 1
    outb(0x1F6, sector >> 24 | 0xE0);// Master drive
    outb(0x1F2, 1); // Read one sector
    outb(0x1F3, sector);
    outb(0x1F4, sector >> 8);
    outb(0x1F5, sector >> 16);
    outb(0x1F7, 0x20); // Make a read call
}

/*
  Inputs:
  addr   -> physical address
  offset -> offset into the file

  Outputs:
  None

  1. Calculate the sector the section lies on
  2. Calculate the physical address to write the sector to
  3. Read sector to physical address
 */
void
ReadProgHeader(uint32_t addr, uint32_t filesz, uint32_t offset)
{
    /* Points to the last address for segment */
    uint32_t end_segment = addr + filesz;

    /* Sector to read */
    uint32_t sect = (offset / SECTOR_SIZE) + KERNEL_START_SECT;

    /* Get to sector boundary */
    addr -= (offset % SECTOR_SIZE);

    for(; addr < end_segment; sect++){
        ReadSector(sect);
        AtaDiskWait();
        insw(0x1F0, (byte *)addr, 512/2);
        addr += SECTOR_SIZE;
    }
}

ELF_HEADER *ReadElfHeader()
{
    /* Read the header to the first stage bootloader */
    ELF_HEADER *elf_head = (ELF_HEADER *)0x5000;

    uint32_t start_sector = 5;

    /* Read the first sector */
    ReadSector(start_sector);
    AtaDiskWait();
    insw(0x1F0, (byte *)elf_head, 512/2);

    /* Confirm its an elf header */
    if(elf_head->ei_magic != ELF_MAGIC){
        return NULL;
    }

    return elf_head;
}

uint32_t ReadKernel()
{
    ELF_HEADER *elf_head = ReadElfHeader();

    if(elf_head  == NULL)
        return NULL;

    /* Validate the number of headers */
    if(elf_head->e_phnum > EXE_MAX_HEADERS || elf_head->e_phnum < 0)
        return NULL;

    /* Get pointer to the first program header */
    ELF_PROG_HEADER *prog_head = (ELF_PROG_HEADER *)((byte *)elf_head + elf_head->e_phoff);
    if (prog_head == 0)
        return NULL;

    /* Read each of the program header */
    for(uint32_t i = 0; i < elf_head->e_phnum; i++){
        /* read each prog_header */
        ReadProgHeader(prog_head->p_paddr, prog_head->p_filesz, prog_head->p_offset);
        /* read next program header */
        prog_head++;
    }

    return elf_head->e_entry;
}

void
SetupKernelPages()
{
    /*
     * Before jumping to the Kernel we need to setup paging for long mode
     * - Clear 5MB memory for kernel page tables
     * - Create Page directory struct from 1MB to 6MB
     * - Identity map first 1MB of memory
     * - Map kernel to high memory (3GB-5GB)
     * - Map page table above kernel
     *
     * At this point we assume our kernel will be of 2 GB.
     * To map 2GB worth of memory we need 1024 PT, 2 PDT, 1 PDPT and 1 PML4T
     * which comes around to just above 4MB so we clear out 5 MB
     * space starting from 1MB to 6MB.
     * To identity map the first MB of memory we need one more PDT and one PT
     * Which will add 12KB of extra memory
     */

    /* Clear Memory
    for (uint32_t *pageAddr = (uint32_t *)KNIX_START_PAGE_ADDR;
         pageAddr < (uint32_t *)KNIX_END_PAGE_ADDR;
         pageAddr++) {
        *pageAddr = 0x0;
    }*/

    //PDT *pdt = (PDT *)KNIX_START_PAGE_ADDR;
    PDT *pdt = (PDT *)0x100000;

    /* Allocate space for 4  PDT */
    PT *pt   = (PT *)(pdt + 1);

#if 0
    /*
     * Identity map the first MB. Each PT has 512 entries mapping 4KB each.
     * This means we have to fill the first 256 entries with the 4KB incremental addresseses.
     */
    uint64_t vAddress = 0x00 | 0x3;

    pdpt->pdpe[0].ui64pdpEntry    = (uint32_t)pdt | 0x3;
    pdt->pde[0].ui64pdEntry       = (uint32_t)pt  | 0x3;

    for(uint32_t pteIdx = 0; pteIdx < 256; pteIdx++){
        pt->pte[pteIdx].ui64ptEntry = vAddress;
        vAddress  += 0x1000;
    }

    /* The kernel will be mapped from 64GB to 66GB */
    vAddress = KERNEL_START_PADDR | 0x3;

    pdt++;
    pdpt->pdpe[63].ui64pdpEntry = (uint32_t)pdt | 0x3;

    /* Map the 1 GB to 64GB-65GB address */
    for (uint32_t pdtIdx = 0; pdtIdx < 512; pdtIdx++) {
        pt++;
        pdt->pde[pdtIdx].ui64pdpEntry = (uint32_t)pt | 0x3;
        for (uint32_t ptIdx = 0; ptIdx < 512; ptIdx++) {
            pt->pte[ptIdx].ui64ptEntry = vAddress;
            vAddress += 0x1000;
        }
    }

    pdt++;
    pdpt->pdpe[64].ui64pdpEntry = (uint32_t)pdt | 0x3;

    /* Map the 1 GB to 4GB-5GB address */
    for (uint32_t pdtIdx = 0; pdtIdx < 512; pdtIdx++) {
        pt++;
        pdt->pde[pdtIdx].ui64pdpEntry = (uint32_t)pt | 0x3;
        for (uint32_t ptIdx = 0; ptIdx < 512; ptIdx++) {
            pt->pte[ptIdx].ui64ptEntry = vAddress;
            vAddress += 0x1000;
        }
    }

#else
    /* Identity mapping 4GB for testing
     * To identity map 4GB worth of memory we need:
     * 1 Page Directory Table (4KB)
     * 1024 Page Tables (4 MB)
     * This comes to just over 4MB. The page tables will reside on memory
     * 1M - 5M
     */
    uint32_t vAddress = 0x00;

    for (uint32_t pdtIdx = 0; pdtIdx < ENTRIES_PER_TABLE; pdtIdx++){
        for(uint32_t pteIdx = 0; pteIdx < ENTRIES_PER_TABLE; pteIdx++){
            pt->pte[pteIdx].ui32ptEntry = vAddress | 0x3;
            vAddress  += 0x1000;
        }
        pdt->pde[pdtIdx].ui32pdEntry = (uint32_t)pt | 0x3;
        pt++;
    }

#endif
}

void
SetupLongModeKernelPaging()
{
    SetupKernelPages();
    SetupPagingAsm();
}

void SecondStageMain(uint32_t mmapAddr)
{
    uint32_t kEntry = 0;

    /* Setup Paging first for correct offset translation of kernel */
    //SetupLongModeKernelPaging();

    /* Setup GDT */
    //LoadGDTAsm();

    /* Read kernel now that paging is setup */
    if((kEntry = ReadKernel()) == NULL) {
        PrintString("Error reading Kernel :(");
        asm("hlt");
    }

    /* We are now ok to long jump to kernel */
    asm volatile("push $0x8\n"
                 "push %0\n"
                 "retf\n":: "r"(kEntry):);
}
