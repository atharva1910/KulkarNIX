Current directory is /home/qwn/Documents/KulkarNIX/Bootloader/SecondStage/
GNU gdb (Ubuntu 8.1-0ubuntu3) 8.1.0.20180409-git
Copyright (C) 2018 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word".
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.0x000000000000fff0 in ?? ()
(gdb) symbol-file IIStageBootloader.
IIStageBootloader.: No such file or directory.
(gdb) cd ../../Build
Working directory /home/qwn/Documents/KulkarNIX/Build.
(gdb) symbol-file IIStageBootloader.sym
Reading symbols from IIStageBootloader.sym...done.
(gdb) b read_kernel
Breakpoint 1 at 0x7fdd: file boot_main.c, line 66.
(gdb) c
Continuing.

Breakpoint 1, read_kernel () at boot_main.c:66
66	{
(gdb) n
read_kernel () at boot_main.c:67
67	    BOOL bRet = false;
(gdb) n
69	    ELF_HEADER *elf_head = read_elf_header();
(gdb) 
read_elf_header () at boot_main.c:46
46	{
(gdb) skip read_sector
Function read_sector will be skipped when stepping.
(gdb) skip ata_disk_wait
Function ata_disk_wait will be skipped when stepping.
(gdb) n
read_elf_header () at boot_main.c:47
47	    ELF_HEADER *elf_head = (ELF_HEADER *)0x10000;
(gdb) 
49	    uint32_t start_sector = 5;
(gdb) 
52	    read_sector(start_sector);
(gdb) 
read_sector (sector=16777216) at boot_main.c:9
9	{
(gdb) 
read_sector (sector=9610498) at boot_main.c:10
10	    ata_disk_wait(); // wait BSY to 0 and RDY to 1
(gdb) 
ata_disk_wait () at boot_main.c:4
4	{
(gdb) 
ata_disk_wait () at boot_main.c:5
5	    while((inb(0x1F7) & 0xC0) != 0x40);
(gdb) 
inb (port=0) at x86.h:13
13	{
(gdb) 
inb (port=0) at x86.h:14
14	    uint8_t data = 0;
(gdb) 
15	    asm volatile("inb %1, %0":"=a"(data):"d"(port));
(gdb) 
16	    return data;
(gdb) 
17	}
(gdb) 
ata_disk_wait () at boot_main.c:6
6	}
(gdb) 
read_sector (sector=9610498) at boot_main.c:11
11	    outb(0x1F6, sector >> 24 | 0xE0);// Master drive
(gdb) 
outb (port=32409, command=180 '\264') at x86.h:7
7	{
(gdb) 
outb (port=502, command=224 '\340') at x86.h:8
8	    asm volatile("outb %0, %1":: "a"(command), "d"(port));
(gdb) 
9	}
(gdb) 
read_sector (sector=9610498) at boot_main.c:12
12	    outb(0x1F2, 1); // Read one sector
(gdb) 
outb (port=31676, command=246 '\366') at x86.h:7
7	{
(gdb) 
outb (port=498, command=1 '\001') at x86.h:8
8	    asm volatile("outb %0, %1":: "a"(command), "d"(port));
(gdb) 
9	}
(gdb) 
read_sector (sector=9610498) at boot_main.c:13
13	    outb(0x1F3, sector);
(gdb) 
outb (port=31676, command=242 '\362') at x86.h:7
7	{
(gdb) 
outb (port=499, command=5 '\005') at x86.h:8
8	    asm volatile("outb %0, %1":: "a"(command), "d"(port));
(gdb) 
9	}
(gdb) 
read_sector (sector=9610498) at boot_main.c:14
14	    outb(0x1F4, sector >> 8);
(gdb) 
outb (port=31676, command=243 '\363') at x86.h:7
7	{
(gdb) 
outb (port=500, command=0 '\000') at x86.h:8
8	    asm volatile("outb %0, %1":: "a"(command), "d"(port));
(gdb) 
9	}
(gdb) 
read_sector (sector=9610498) at boot_main.c:15
15	    outb(0x1F5, sector >> 16);
(gdb) 
outb (port=31676, command=244 '\364') at x86.h:7
7	{
(gdb) 
outb (port=501, command=0 '\000') at x86.h:8
8	    asm volatile("outb %0, %1":: "a"(command), "d"(port));
(gdb) 
9	}
(gdb) 
read_sector (sector=9610498) at boot_main.c:17
17	    outb(0x1F7, 0x20);
(gdb) 
outb (port=31676, command=245 '\365') at x86.h:7
7	{
(gdb) 
outb (port=503, command=32 ' ') at x86.h:8
8	    asm volatile("outb %0, %1":: "a"(command), "d"(port));
(gdb) 
9	}
(gdb) 
read_sector (sector=9610498) at boot_main.c:18
18	}
(gdb) 
read_elf_header () at boot_main.c:53
53	    ata_disk_wait();
(gdb) 
ata_disk_wait () at boot_main.c:4
4	{
(gdb) 
ata_disk_wait () at boot_main.c:5
5	    while((inb(0x1F7) & 0xC0) != 0x40);
(gdb) 
inb (port=32) at x86.h:13
13	{
(gdb) 
inb (port=503) at x86.h:14
14	    uint8_t data = 0;
(gdb) 
15	    asm volatile("inb %1, %0":"=a"(data):"d"(port));
(gdb) 
16	    return data;
(gdb) 
17	}
(gdb) 
ata_disk_wait () at boot_main.c:6
6	}
(gdb) 
read_elf_header () at boot_main.c:54
54	    insw(0x1F0, (BYTE *)elf_head, 512/2);
(gdb) 
insw (port=31676, address=0x100 <incomplete sequence \354>, count=16777216) at x86.h:21
21	{
(gdb) 
insw (port=32, address=0x1f0 "S\377", count=65536) at x86.h:22
22	    asm volatile("cld; rep insw":"+D"(address), "+c"(count):"d"(port): "memory");
(gdb) 
23	}
(gdb) 
read_elf_header () at boot_main.c:57
57	    if(elf_head->ei_magic != ELF_MAGIC){
(gdb) 
61	    return elf_head;
(gdb) p elf_head
$1 = (ELF_HEADER *) 0x92a502
(gdb) p/x elf_head
$2 = 0x92a502
(gdbx/10x elf_head
No symbol "elf_j" in current context.
(gdb) x/10x elf_head
0x92a502:	0x00000000	0x00000000	0x00000000	0x00000000
0x92a512:	0x00000000	0x00000000	0x00000000	0x00000000
0x92a522:	0x00000000	0x00000000
(gdb) x/10x 0x10000
0x10000:	0x464c457f	0x00010101	0x00000000	0x00000000
0x10010:	0x00030002	0x00000001	0x00100000	0x00000034
0x10020:	0x000021c0	0x00000000
(gdb) p *elf_head
$3 = {ei_magic = 0, ei_class = 0 '\000', ei_data = 0 '\000', ei_version = 0 '\000', ei_osabi = 0 '\000', ei_abiver = 0 '\000', ei_pad = "\000\000\000\000\000\000", e_type = 0, e_machine = 0, e_version = 0, e_entry = 0, e_phoff = 0, e_shoff = 0, e_flags = 0, e_ehsize = 0, e_phentsize = 0, e_phnum = 0, e_shentsize = 0, e_shnum = 0, e_shstrndx = 0}
(gdb) n
62	}
(gdb) 
read_kernel () at boot_main.c:71
71	    if(elf_head  == NULL)
(gdb) p elf_head
$4 = (ELF_HEADER *) 0x7fec <read_kernel+15>
(gdb) p *elf_head
$5 = {ei_magic = 2213823881, ei_class = 125 '}', ei_data = -12 '\364', ei_version = 0 '\000', ei_osabi = 117 'u', ei_abiver = 9 '\t', ei_pad = "\017\266E\373", <incomplete sequence \351\201>, e_type = 0, e_machine = 17803, e_version = 1085738996, e_entry = 4169360940, e_phoff = 3087496707, e_shoff = 0, e_flags = 1166765547, e_ehsize = 35828, e_phentsize = 7248, e_phnum = 17803, e_shentsize = 500, e_shnum = 35280, e_shstrndx = 64581}
(gdb) 10x 0x10000
Undefined command: "10x".  Try "help".
(gdb) n
75	    if(elf_head->e_phnum > EXE_MAX_HEADERS || elf_head->e_phnum < 0)
(gdb) 
79	    ELF_PROG_HEADER *prog_head = (ELF_PROG_HEADER *)((BYTE *)elf_head + elf_head->e_phoff);
(gdb) p prog_head
$6 = (ELF_PROG_HEADER *) 0x10000
(gdb) s
80	    if (prog_head == 0)
(gdb) p prog_head
$7 = (ELF_PROG_HEADER *) 0x10000
(gdb) s
84	    ELF_PROG_HEADER *last_prog_head = (ELF_PROG_HEADER *)(prog_head + elf_head->e_phnum);
(gdb) p last_prog_head
$8 = (ELF_PROG_HEADER *) 0x7bf0
(gdb) s
85	    if (last_prog_head == 0)
(gdb) p last_prog_head
$9 = (ELF_PROG_HEADER *) 0x7bf0
(gdb) p prog_head
$10 = (ELF_PROG_HEADER *) 0x10000
(gdb) p elf_head
$11 = (ELF_HEADER *) 0x7fec <read_kernel+15>
(gdb) p/x elf_head->e_phoff
$12 = 3087496707
(gdb) p/x elf_head->e_phoff
$13 = 0xb8077603
(gdb) n
91	        read_prog_header(prog_head->p_paddr, prog_head->p_filesz, prog_head->p_offset);
(gdb) 
read_prog_header (addr=96, filesz=4096, offset=65652) at boot_main.c:33
33	{
(gdb) n
read_prog_header (addr=31728, filesz=32875, offset=1048576) at boot_main.c:34
34	    uint32_t end_segment = addr + offset; // Points to the last address for segment
(gdb) p end_segment
$14 = 496
(gdb) n
35	    uint32_t sect        = (offset / SECTOR_SIZE) + KERNEL_START_SECT; // Sector to read
(gdb) p end_segment
$15 = 496
(gdb) p/x end_segment
$16 = 0x1f0
(gdb) p/x addr
$17 = 0x7bf0
(gdb) p/x filesz
$18 = 0x806b
(gdb) p/x offset
$19 = 0x100000
(gdb) n
37	    for(; addr < end_segment; sect++){
(gdb) 
38	        read_sector(sect);
(gdb) p sect
$20 = 66048
(gdb) p/x sect
$21 = 0x10200
(gdb) n
read_sector (sector=496) at boot_main.c:9
9	{
(gdb) p sector
$22 = 496
(gdb) p/x sector
$23 = 0x1f0
(gdb) n
read_sector (sector=66048) at boot_main.c:10
10	    ata_disk_wait(); // wait BSY to 0 and RDY to 1
(gdb) 
ata_disk_wait () at boot_main.c:4
4	{
(gdb) 
ata_disk_wait () at boot_main.c:5
5	    while((inb(0x1F7) & 0xC0) != 0x40);
(gdb) 
inb (port=0) at x86.h:13
13	{
(gdb) 
inb (port=0) at x86.h:14
14	    uint8_t data = 0;
(gdb) 
15	    asm volatile("inb %1, %0":"=a"(data):"d"(port));
(gdb) 
16	    return data;
(gdb) 
17	}
(gdb) 
ata_disk_wait () at boot_main.c:6
6	}
(gdb) 
read_sector (sector=66048) at boot_main.c:11
11	    outb(0x1F6, sector >> 24 | 0xE0);// Master drive
(gdb) p sector
$24 = 66048
(gdb) p/x sector
$25 = 0x10200
(gdb) q
Detaching from program: , Remote target
Ending remote debugging.

Debugger finished
