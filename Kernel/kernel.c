typedef char BYTE;

void
print_char(char *address, char c, BYTE bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}

void
print_string(char *string)
{
    BYTE bgcolor = 0x07;
    char *vga_buffer = (char *)0xb8000;
    char c = 0;
    int pos = 0;
    while((c = string[pos++]) != '\0'){
        print_char(vga_buffer, c, 0x07);
        vga_buffer += 2;
    }
}


void
kernel_main()
{
    char *c = "Welcome to the kernel";
    print_string(c);
    while(1);
}
