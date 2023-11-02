#ifndef _CPU_H
#define _CPU_H
#include <stdio.h>
#include <stdint.h>
#define DRAM_BASE 0x80000000ull
#define DRAM_SIZE 128*1024*1024ull //128M
#ifndef DRAM_OFFSET
#define DRAM_OFFSET 0
#endif
#define RESET_ADDRESS  DRAM_BASE+DRAM_OFFSET
typedef struct DRAM{
    uint8_t* dram;
}DRAM;
typedef struct BUS
{
    DRAM dram;
}BUS;
typedef enum CPU_PRI_LEVE{
    U=0B00,
    S=0B01,
    M=0B11,
}CPU_PRI_LEVE;
typedef enum CPU_STATE{
    CPU_START,
    CPU_RUN,
    CPU_STOP,
}CPU_STATE;
typedef struct CSR{
    uint32_t csr[4096];
}CSR;
typedef struct CPU
{
    uint32_t reg[32];
    uint32_t PC;
    BUS bus;
    CPU_PRI_LEVE pri_level;
    CPU_STATE cpu_state;
    CSR csr;
}CPU;

enum INST_NAME{
    //RV32I
    ADD,
    ADDI,
    AND,
    ANDI,
    AUIPC,
    BEQ,
    BGE,
    BGEU,
    BLT,
    BLTU,
    BNE,
    EBRERAK,
    ECALL,
    MRET,
    FENCE,
    JAL,
    JALR,
    LB,
    LBU,
    LH,
    LHU,
    LUI,
    LW,
    OR,
    ORI,
    SB,
    SH,
    SLL,
    SLT,
    SLTI,
    SLTIU,
    SLTU,
    SRA,
    SRL,
    SRLI,
    SRAI,
    SUB,
    SW,
    XOR,
    XORI,
    SLLI,
    //ZICSR
    CSRRC,
    CSRRCI,
    CSRRS,
    CSRRSI,
    CSRRW,
    CSRRWI,
    INST_NUM,
};

typedef struct rv_csrs {
  uint32_t mhartid, mstatus, mstatush, mscratch, mepc, mcause, mtval, mip, mtinst,
      mtval2, mtvec, mie;
} rv_csrs;

typedef struct DECODER{
    uint32_t inst;//introduction
    enum INST_NAME inst_name;//name of introduction
    uint32_t dest;
    uint32_t imm; //imm
    uint32_t csr_addr;
    int rd;
    int rs1;
    int rs2;
    int shamt;
    CPU *cpu;
    uint32_t snpc;
    uint32_t dnpc;//next pc
    rv_csrs rv_csr;
}DECODER;

extern CPU cpu;
extern uint64_t MASK(int n);
extern uint64_t BITS(uint64_t imm, int hi, int lo);
extern uint32_t imm_i(uint32_t inst);
extern void dram_init(DRAM *dram);
extern void cpu_init(CPU *cpu);
extern void init_inst_funct();
extern CPU cpu;
extern int cpu_exec(CPU *cpu, uint32_t n);

enum CSR_ADDR{
    mpec = 0x341,
    mcause = 0x342,
    mtvec = 0x305,

};

// #define MTVEC_ADDR 0x305
// #define MCAUSE_ADDR 0X341
void printf_regs();
#endif