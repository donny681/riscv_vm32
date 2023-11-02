    .macro init;
    .endm
    .section .init;
    .option norvc
    .option nopic
    .align  4;
    .weak reset_vector;
    .globl __bss_start;
    .globl __bss_end;
    .type _start,@function
    #define pdev_uart0_write_addr   1

_start:
    la sp, _heap_start
    addi a0,x0,'S'
    sb a0,1(x0)
    /* reset vector */
    j reset_vector

.align  4;
reset_vector:
	/* Clear bss section */
	la a1, __bss_start
	la a2, __bss_end
	bgeu a1, a2, 2f
1:
	sw zero, 0(a1)
	addi a1, a1, 4
	bltu a1, a2, 1b
2:

    call main;
    # mret;
    unimp

loop:
    j loop;