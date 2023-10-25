ENTRY(__start)

SECTIONS
{
    . = 0x800000;

    /*
        . = 0xC0000000;
        Align the segemts to each page size so that it's easier to specify permissions
    */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(.bss)
    }

}
