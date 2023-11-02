#include <stdio.h>
#include "../include/cpu.h"
#include "elog.h"
void print_regs(){
    for(int i = 0;i<32;i++){
        log_e("reg[%d] = %lx", i ,cpu.reg[i]);
    }
}