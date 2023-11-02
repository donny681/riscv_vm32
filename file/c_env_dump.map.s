
c_env.elf:     file format elf32-littleriscv
c_env.elf
architecture: riscv:rv32, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x80000000

Program Header:
0x70000003 off    0x000013a5 vaddr 0x00000000 paddr 0x00000000 align 2**0
         filesz 0x0000001c memsz 0x00000000 flags r--
    LOAD off    0x00001000 vaddr 0x80000000 paddr 0x80000000 align 2**12
         filesz 0x00000058 memsz 0x00000058 flags rwx
    LOAD off    0x00000000 vaddr 0x80017000 paddr 0x80017000 align 2**12
         filesz 0x00000000 memsz 0x00002000 flags rw-

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         00000058  80000000  80000000  00001000  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         00000000  80000058  80000058  00001058  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000000  80000058  80000058  00001058  2**0
                  ALLOC
  3 .stack        00002000  80017000  80017000  00002000  2**0
                  ALLOC
  4 .debug_info   00000078  00000000  00000000  00001058  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 0000005f  00000000  00000000  000010d0  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 00000040  00000000  00000000  00001130  2**3
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_rnglists 00000016  00000000  00000000  00001170  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_line   000000ed  00000000  00000000  00001186  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_str    000000f4  00000000  00000000  00001273  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .debug_line_str 0000002e  00000000  00000000  00001367  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 11 .comment      00000010  00000000  00000000  00001395  2**0
                  CONTENTS, READONLY
 12 .riscv.attributes 0000001c  00000000  00000000  000013a5  2**0
                  CONTENTS, READONLY
 13 .debug_frame  00000020  00000000  00000000  000013c4  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS
SYMBOL TABLE:
80000000 l    d  .text	00000000 .text
80000058 l    d  .data	00000000 .data
80000058 l    d  .bss	00000000 .bss
80017000 l    d  .stack	00000000 .stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_rnglists	00000000 .debug_rnglists
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .debug_line_str	00000000 .debug_line_str
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .riscv.attributes	00000000 .riscv.attributes
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    df *ABS*	00000000 start.o
80000000 l     F .text	00000000 _start
80000048 l       .text	00000000 loop
00000000 l    df *ABS*	00000000 test.c
00002000 g       *ABS*	00000000 __stack_size
80000058 g       .stack	00000000 __bss_end
80000020  w      .text	00000000 reset_vector
80019000 g       .stack	00000000 _heap_start
00000000         *UND*	00000000 _start
80000058 g       .bss	00000000 __bss_start
80000050 g     F .text	00000008 main


