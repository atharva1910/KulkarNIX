ENTRY(__start)

SECTIONS
{
    . = 0x8000;

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

    .rodata : ALIGN(0)
    {
        *(.rodata)
    }

}
