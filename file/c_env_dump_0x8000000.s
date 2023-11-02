
c_env.elf:     file format elf32-littleriscv

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         0000005c  80000000  80000000  00001000  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         00000000  8000005c  8000005c  0000105c  2**0
                  CONTENTS, ALLOC, LOAD, DATA

Disassembly of section .text:

80000000 <_start>:
    .globl __bss_end;
    .type _start,@function
    #define pdev_uart0_write_addr   1

_start:
    la sp, _heap_start
80000000:	00019117          	auipc	sp,0x19
80000004:	00010113          	mv	sp,sp
    addi a0,x0,'S'
80000008:	05300513          	li	a0,83
    sb a0,1(x0)
8000000c:	00a000a3          	sb	a0,1(zero) # 1 <__stack_size-0x1fff>
    /* reset vector */
    j reset_vector
80000010:	0100006f          	j	80000020 <reset_vector>
80000014:	00000013          	nop
80000018:	00000013          	nop
8000001c:	00000013          	nop

80000020 <reset_vector>:

.align  4;
reset_vector:
	/* Clear bss section */
	la a1, __bss_start
80000020:	00000597          	auipc	a1,0x0
80000024:	03c58593          	add	a1,a1,60 # 8000005c <__bss_end>
	la a2, __bss_end
80000028:	00000617          	auipc	a2,0x0
8000002c:	03460613          	add	a2,a2,52 # 8000005c <__bss_end>
	bgeu a1, a2, 2f
80000030:	00c5f863          	bgeu	a1,a2,80000040 <reset_vector+0x20>
1:
	sw zero, 0(a1)
80000034:	0005a023          	sw	zero,0(a1)
	addi a1, a1, 4
80000038:	00458593          	add	a1,a1,4
	bltu a1, a2, 1b
8000003c:	fec5ece3          	bltu	a1,a2,80000034 <reset_vector+0x14>
2:

    j main;
80000040:	0140006f          	j	80000054 <main>
    # mret;
    unimp
80000044:	c0001073          	unimp

80000048 <loop>:

loop:
80000048:	0000006f          	j	80000048 <loop>
8000004c:	0000                	.2byte	0x0
8000004e:	0000                	.2byte	0x0
80000050:	0000                	.2byte	0x0
80000052:	0000                	.2byte	0x0

80000054 <main>:
int main(){
    // while(1){
        int a = 0;
    // }
    return 0;
80000054:	00000513          	li	a0,0
80000058:	00008067          	ret
