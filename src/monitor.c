
#include "../include/monitor.h"
#include "../include/cpu.h"
#include<assert.h>
char opt[MAX_OPTION_LENGTH];
char img_path[MAX_IMG_PATH_LENGTH];
void cmd_c() {
    cpu_exec(&cpu, -1);
}
void cmd_h() {
    puts("q: quit");
    puts("c: run");
    puts("r path: read .bin file from path to memory");
    puts("s: exec once");
    puts("h: help");
}
void cmd_r() {
    scanf("%s", img_path);
    FILE *fp = fopen(img_path, "rb");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    printf("Read %ld byte from file.\n", size);
    fseek(fp, 0, SEEK_SET);
    fread(cpu.bus.dram.dram + DRAM_OFFSET, size, 1, fp);
    fclose(fp);
}
void cmd_s() {
    exec_once(&cpu);
}
void cmd_re() {
    for (int i = 0; i < 32; i++) {
        printf("reg #%d == 0x%08x\n", i, cpu.reg[i]);
    }
}


void read_file(const char *file_path){
    
    FILE *fp = fopen(file_path, "rb");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    printf("Read %ld byte from file.\n", size);
    fseek(fp, 0, SEEK_SET);
    fread(cpu.bus.dram.dram + DRAM_OFFSET, size, 1, fp);
    fclose(fp);
}