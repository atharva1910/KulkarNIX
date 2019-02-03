extern "C" void PPrintString();

extern "C" void
boot_main()
{
    PPrintString();
    while(1);
}
