ENTRY(__start)

SECTIONS
{
    . = 0x500;

    /*
        Align the segemts to each page size so that it's easier to specify permissions
    */
    .text : ALIGN(0)
    {
        *(.text)
    }

    .data : ALIGN(0)
    {
        *(.data)
    }

    .bss : ALIGN(0)
    {
        *(.bss)
    }

}
