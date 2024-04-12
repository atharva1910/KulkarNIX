#define KINTR(x) 						\
    __asm__(    						\
    ".section .text\n"      			\
    ".global KInterrupt\n"              \
    ".type KInterrupt, @function\n"     \
    "KInterrupt:\n"                     \
    "call KInterruptInt\n"              \
    "iret");                            \

#define BAK_KINTR(x)                   \
    __asm__(    						\
    ".section .text\n"      			\
    ".global KInterrupt##x\n"           \
    ".type KInterrupt##x, @function\n"  \
    "KInterrupt##x:\n"                  \
    "call KInterruptInt\n"              \
    "iret");                            \


void KInterruptInt()
{

    __asm__("hlt");
}

KINTR(1);
