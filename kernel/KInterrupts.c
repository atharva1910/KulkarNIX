#define BAK_KINTR(x) 	     			\
    __asm__(    						\
    ".section .text\n"      			\
    ".global KInterrupt\n"              \
    "KInterrupt:\n"                     \
    "call KInterruptInt\n"              \
    "iret");

#define KINTR(x)                        \
    __asm__(    						\
    ".section .text\n"      			\
    ".global KInterrupt"#x"\n"          \
    ".type KInterrupt"#x", @function\n" \
    "KInterrupt"#x":\n"                 \
    "call KInterruptInt\n"              \
    "iret");                            \

#define temp(num)  \
    void intr##num(



void KInterruptInt(int intr)
{

    __asm__("hlt");
}

KINTR(1);
KINTR(2);
KINTR(3);
