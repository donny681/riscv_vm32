

c_env.elf:     file format elf32-littleriscv

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         00000054  00000000  00000000  00001000  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         00000000  00000054  00000054  00001054  2**0
                  CONTENTS, ALLOC, LOAD, DATA

Disassembly of section .text:

00000000 <_start>:
    .globl __bss_end;
    .type _start,@function
    #define pdev_uart0_write_addr   1

_start:
    la sp, _heap_start
   0:	00019117          	auipc	sp,0x19
   4:	00010113          	mv	sp,sp
    addi a0,x0,'S'
   8:	05300513          	li	a0,83
    sb a0,1(x0)
   c:	00a000a3          	sb	a0,1(zero) # 1 <_start+0x1>
    /* reset vector */
    j reset_vector
  10:	0100006f          	j	20 <reset_vector>
  14:	00000013          	nop
  18:	00000013          	nop
  1c:	00000013          	nop

00000020 <reset_vector>:

.align  4;
reset_vector:
	/* Clear bss section */
	la a1, __bss_start
  20:	05400593          	li	a1,84
	la a2, __bss_end
  24:	05400613          	li	a2,84
	bgeu a1, a2, 2f
  28:	00c5f863          	bgeu	a1,a2,38 <reset_vector+0x18>
1:
	sw zero, 0(a1)
  2c:	0005a023          	sw	zero,0(a1)
	addi a1, a1, 4
  30:	00458593          	add	a1,a1,4
	bltu a1, a2, 1b
  34:	fec5ece3          	bltu	a1,a2,2c <reset_vector+0xc>
2:

    j main;
  38:	0140006f          	j	4c <main>
    # mret;
    unimp
  3c:	c0001073          	unimp

00000040 <loop>:

loop:
  40:	0000006f          	j	40 <loop>
  44:	0000                	.2byte	0x0
  46:	0000                	.2byte	0x0
  48:	0000                	.2byte	0x0
  4a:	0000                	.2byte	0x0

0000004c <main>:
int main(){
    // while(1){
        int a = 0;
    // }
    return 0;
  4c:	00000513          	li	a0,0
  50:	00008067          	ret
