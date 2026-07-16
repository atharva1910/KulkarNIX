ENTRY(__start)

SECTIONS
{
    /*
     * The page tables are located before the kernel, they are worth 2 MB
     * So 0xC0000000 - 0xC02FFFFF are the page tables (3GB - 3GB + 2MB)
     */
    . = 0xC0300000;

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata*)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(.bss)
        *(COMMON)
    }

}
