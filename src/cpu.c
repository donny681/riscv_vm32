#include <stdio.h>

#include <stdlib.h>
#include "elog.h"
#include "../include/cpu.h"
#include "other.h"
CPU cpu;
void (*inst_handler[INST_NUM])(DECODER *);

uint64_t MASK(int n)
{
    if (n == 64)
        return -1;
    return (1ull << n) - 1;
}
uint64_t BITS(uint64_t imm, int hi, int lo)
{
    // log_d("Hello EasyLogger!");
    log_d("BITS:%u!,imm:%x,hi:%d,lo:%d", (imm >> lo) & MASK(hi - lo + 1), imm, hi, lo);
    return (imm >> lo) & MASK(hi - lo + 1);
}
uint64_t SEXT(uint64_t imm, int n)
{
    log_d("SEXT:%u!,MASK(64) << n =%x,imm:%lu,n:%d", (MASK(64) << n) & imm, (MASK(64) << n), imm, n);
    return ((MASK(64) << n) & imm);
}

#define rv_bf(i, h, l) \
    (((i) >> (l)) & ((1 << ((h) - (l) + 1)) - 1))  /* extract bit field */
#define rv_b(i, l) rv_bf(i, l, l)                  // extract bit
#define rv_tb(i, l, o) (rv_b(i, l) << (o))         /* translate bit */
#define rv_tbf(i, h, l, o) (rv_bf(i, h, l) << (o)) // translate bit field

uint32_t rv_signext(uint32_t x, uint32_t h)
{ /* sign-extend x from h'th bit */
    log_d("x=%x,h=%x,x >> h=%x,0 - (x >> h)=%d,(0 - (x >> h)) << h =%d\r\n", x, h, x >> h, 0 - (x >> h), (0 - (x >> h)) << h);
    return (0 - (x >> h)) << h | x;
}

#define rv_iimm_i(i) rv_signext(rv_bf(i, 31, 20), 11) /* imm. for I-type */
#define rv_iimm_u(i) rv_tbf(i, 31, 12, 12)            // imm for U-type
#define rv_iimm_b(i)                                                             \
    (rv_signext(rv_tb(i, 31, 12), 12) | rv_tb(i, 7, 11) | rv_tbf(i, 30, 25, 5) | \
     rv_tbf(i, 11, 8, 1)) /* imm. for B-type */
#define rv_iimm_j(i)                                            \
    (rv_signext(rv_tb(i, 31, 20), 20) | rv_tbf(i, 19, 12, 12) | \
     rv_tb(i, 20, 11) | rv_tbf(i, 30, 21, 1)) /* imm. for J-type */
// #define rv_iimm_s(i) (rv_signext(rv_tbf(i,31,25,5) | rv_bf(i,31,25)),11) /* imm. for S-type */
#define rv_iimm_s(i)                                               \
    (rv_signext(rv_tbf(i, 31, 25, 5), 11) | rv_tbf(i, 30, 25, 5) | \
     rv_bf(i, 11, 7)) /* imm. for S-type */
void dram_init(DRAM *dram)
{
    dram->dram = (uint8_t *)malloc(DRAM_SIZE);
    assert(dram->dram);
}

void dram_store(DRAM *dram, uint32_t addr, int length, uint32_t val)
{
    log_d("dram store 0x%lx", addr);
    assert(length == 1 || length == 2 || length == 4 || length == 8);
    assert(addr >= 0 && addr < DRAM_SIZE);
    switch (length)
    {
    case 1:
        dram->dram[addr] = val & 0xff;
        return;
    case 2:
        dram->dram[addr] = val & 0xff;
        dram->dram[addr + 1] = (val & 0xff00) >> 8;
        return;
    case 4:
        dram->dram[addr] = val & 0xff;
        dram->dram[addr + 1] = (val & 0xff00) >> 8;
        dram->dram[addr + 2] = (val & 0xff0000) >> 16;
        dram->dram[addr + 3] = (val & 0xff000000) >> 24;
        return;
    default:
        break;
    }
}

uint32_t dram_load(DRAM *dram, uint32_t addr, int length)
{
    log_d("addr = %x,length=%d", addr, length);
    assert(length == 1 || length == 2 || length == 4);
    assert(addr >= 0 && addr <= DRAM_SIZE);
    switch (length)
    {
    case 1:
        return dram->dram[addr]; // LB
    case 2:
        return (dram->dram[addr + 1] << 8 | dram->dram[addr]); // LH
    case 4:
        return (dram->dram[addr + 3] << 24 | dram->dram[addr + 2] << 16 | dram->dram[addr + 1] << 8 | dram->dram[addr]); // LW
    default:
        break;
    }
    return 0;
}

uint32_t mem_load(DRAM *dram, uint32_t addr, int length)
{
    log_d("mem load addr = 0x%08lx", addr);
    return dram_load(dram, addr - DRAM_BASE, length);
}

void mem_store(DRAM *dram, uint32_t addr, int length, uint32_t val)
{
    dram_store(dram, addr - DRAM_BASE, length, val);
}

void cpu_init(CPU *cpu)
{
    cpu->reg[0] = 0;
    cpu->reg[2] = DRAM_BASE + DRAM_SIZE;
    cpu->PC = RESET_ADDRESS;
    cpu->pri_level = M;
    cpu->cpu_state = CPU_RUN;
}

uint32_t cpu_load(CPU *cpu, uint32_t addr, int length)
{
    return mem_load(&cpu->bus.dram, addr, length);
}
void cpu_store(CPU *cpu, uint32_t addr, int length, uint32_t val)
{

    if (addr == 1)
    {
        // log_e("%c", val);
        putchar(val);
    }
    else
    {
        mem_store(&cpu->bus.dram, addr, length, val);
    }
}

DECODER decode(uint32_t inst)
{
    DECODER ret;
    ret.inst = inst;
    ret.rd = rv_bf(inst, 11, 7);
    ret.rs1 = rv_bf(inst, 19, 15);
    ret.rs2 = rv_bf(inst, 24, 20);
    ret.shamt = rv_bf(inst, 25, 20);
    ret.csr_addr = rv_bf(inst, 31, 20);
    uint32_t funct3 = rv_bf(inst, 14, 12);
    uint32_t funct7 = rv_bf(inst, 31, 25);
    uint32_t funct6 = rv_bf(inst, 31, 26);
    uint32_t opcode = rv_bf(inst, 6, 0);

    log_d("opcode= %x,funct3=%x,funct6=%x , funct7 = %x", opcode, funct3, funct6, funct7);
    switch (opcode)
    {
    case 0b0110011: // R inst
        switch (funct7 << 3 | funct3)
        {
        case 0b000: // ADD
            ret.inst_name = ADD;
            break;
        case 0b100000000: // SUB
            ret.inst_name = SUB;
            break;
        case 0b001: // sll
            ret.inst_name = SLL;
            break;
        case 0b111: // AND
            ret.inst_name = AND;
            break;
        case 0b110: // OR
            ret.inst_name = OR;
            break;
        case 0b010: // SLT
            ret.inst_name = SLT;
            break;
        case 0b011: // SLTU
            ret.inst_name = SLTU;
            break;
        case 0b101: // SRL
            ret.inst_name = SRL;
            break;
        case 0b0100000101: // SRA
            ret.inst_name = SRA;
            break;
        case 0b100: // XOR
            ret.inst_name = XOR;
            break;
        default:
            ret.inst_name = INST_NUM;
            break;
        }
        break;
    case 0b0010011: // I inst
        ret.imm = rv_iimm_i(inst);
        switch (funct3)
        {
        case 0b000: // ADDI
            ret.inst_name = ADDI;
            break;
        case 0b001: // SLLI
            ret.inst_name = SLLI;
            break;
        case 0b010: // SLTI
            ret.inst_name = SLTI;
            break;
        case 0b011: // SLTIU
            ret.inst_name = SLTIU;
            break;
        case 0b101: //
            log_d("SRLI,SRAI");
            switch (funct7)
            {
            case 0:
                ret.inst_name = SRLI; // SRLI
                break;
            case 0b0100000:
                ret.inst_name = SRAI; // SRAI
                break;
            default:
                ret.inst_name = INST_NUM; // SRAI
                break;
            }

            break;
        case 0b111: // ANDI
            ret.inst_name = ANDI;
            break;
        case 0b110: // ORI
            ret.inst_name = ORI;
            break;
        case 0b100: // XORI
            ret.inst_name = XORI;
            break;
        default:
            ret.inst_name = INST_NUM;
            break;
        }
        break;
    case 0b1100011: // B inst
        ret.imm = rv_iimm_b(inst);
        switch (funct3)
        {
        case 0b000: // BEQ
            ret.inst_name = BEQ;
            break;
        case 0b001: // BNE
            ret.inst_name = BNE;
            break;
        case 0b100: // blt
            ret.inst_name = BLT;
            break;
        case 0b101: // bge
            ret.inst_name = BGE;
            break;
        case 0b110: // bltu
            ret.inst_name = BLTU;
            break;
        case 0b111:
            ret.inst_name = BGEU;
            break;
        default:
            ret.inst_name = INST_NUM;
            break;
        }
        break;
    case 0b0100011: // S inst
        ret.imm = rv_iimm_s(inst);
        log_d("ret.imm=%x,inst = %lx", ret.imm, inst);
        switch (funct3)
        {
        case 0b000:
            ret.inst_name = SB;
            break;
        case 0b001:
            ret.inst_name = SH;
            break;
        case 0b010: // SW
            ret.inst_name = SW;
            break;
        default:
            ret.inst_name = INST_NUM;
            break;
        }
        break;
    case 0b0010111: // u inst
        ret.inst_name = AUIPC;
        ret.imm = rv_iimm_u(inst);
        break;
    case 0b1100111: // jalr
        ret.inst_name = JALR;
        ret.imm = rv_iimm_i(inst);
        break;
    case 0b1101111: // J inst
        ret.inst_name = JAL;
        ret.imm = rv_iimm_j(inst);
        break;
    case 0b1110011: // I break
        if (funct3)
        {
            // csr
            switch (funct3)
            {
            case 0b001:
                ret.inst_name = CSRRW;
                break;
            case 0b010:
                ret.inst_name = CSRRS;
                break;
            case 0b011:
                ret.inst_name = CSRRC;
                break;
            case 0b101:
                ret.inst_name = CSRRWI;
                break;
            case 0b110:
                ret.inst_name = CSRRSI;
                break;
            case 0b111:
                ret.inst_name = CSRRCI;
                break;
            default:
                ret.inst_name = INST_NUM;
                break;
            }
        }
        else
        {
            switch ((funct7 << 5) | ret.rs2)
            {
            case 0: // ecall
                ret.inst_name = ECALL;
                break;
            case 1: // ebreak
                ret.inst_name = EBRERAK;
                break;
            case 0b001100000010: // mret
                ret.inst_name = MRET;
                break;
            default:
                ret.inst_name = INST_NUM;
                break;
            }
        }
        break;
    case 0b0000011:
        ret.imm = rv_iimm_i(inst);
        switch (funct3)
        {
        case 0b000: // lb load byte
            ret.inst_name = LB;
            break;
        case 0b001: // lh load half
            ret.inst_name = LH;
            break;
        case 0b010: // lw
            ret.inst_name = LW;
            break;
        case 0b101:
            ret.inst_name = LHU;
            break;
        case 0b100:
            ret.inst_name = LBU; // Load Halfword
            break;
        // case 0b
        default:
            ret.inst_name = INST_NUM;
            break;
        }
        break;
    case 0b0110111: // u inst
        // log_e("!!!!!!!!!LUI");
        ret.inst_name = LUI;
        ret.imm = rv_iimm_u(inst);
        break;
    case 0b0001111:
        switch (funct3)
        {
        case 0:
            ret.inst_name = FENCE;
            break;
        default:
            ret.inst_name = INST_NUM;
            break;
        }
        break;
    default:
        log_e("opcode error\r\n");
        ret.inst_name = INST_NUM;
        break;
    }
    return ret;
}

void set_inst_func(enum INST_NAME inst_name, void (*fp)(DECODER *))
{
    inst_handler[inst_name] = fp;
}
/*
R inst ----ADD
rd = rs1 + rs2
*/
void add(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] + decoder->cpu->reg[decoder->rs2];
}

/*
I inst ----ADDI
rd = rs1 + imm
*/
void addi(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] + decoder->imm;
    log_d("addi rd = %d , x[decoder->rd]=%lx ,  x[rs1 = %d] = 0x%lx imm = 0x%lx\n", decoder->rd, decoder->cpu->reg[decoder->rd], decoder->rs1, decoder->cpu->reg[decoder->rs1], decoder->imm);
    log_d("addi rd =%d ,rs1=%d,imm=%lx , %lx = %lx +%lx\n", decoder->rd, decoder->rs1, decoder->imm, decoder->cpu->reg[decoder->rd], decoder->cpu->reg[decoder->rs1],decoder->imm);
}

/*
R inst ----AND
rd = rs1 & rs2
*/
void and (DECODER * decoder)
{
    log_d("and rd = %d x[rs1 = %d] = 0x%lx,x[rs2 = %d] = 0x%lx", decoder->rd, decoder->rs1, decoder->cpu->reg[decoder->rs1], decoder->rs2, decoder->cpu->reg[decoder->rs2]);
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] & decoder->cpu->reg[decoder->rs2];
}

/*
I inst ----ANDI
rd= rs1 & imm
*/
void andi(DECODER *decoder)
{
    log_d("andi rd = %d x[rs1 = %d] = 0x%lx imm = 0x%lx\n", decoder->rd, decoder->rs1, decoder->cpu->reg[decoder->rs1], decoder->imm);
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] & decoder->imm;
}

/*
U inst ----AUIPC
rd = pc + imm<<12
*/
void auipc(DECODER *decoder)
{
    log_d("auipc rd = %d, pc = 0x%lx, decoder->imm= 0x%lx", decoder->rd, decoder->cpu->PC, decoder->imm);
    decoder->cpu->reg[decoder->rd] = decoder->cpu->PC + decoder->imm;
}

/*
B inst ----beq
imm[12|10:5] rs2 rs1 000 imm[4:1|11] 1100011
pc = rs1 == rs2 ? pc + imm, pc + 4
*/
void beq(DECODER *decoder)
{
    if (decoder->cpu->reg[decoder->rs1] == decoder->cpu->reg[decoder->rs2])
    {
        log_d("beq offset = 0x%lx", decoder->imm);
        decoder->dnpc = decoder->cpu->PC + decoder->imm;
    }
}
/*
B inst ----bge
imm[12|10:5] rs2 rs1 101 imm[4:1|11] 1100011
pc = rs1 >= rs2 ? pc + imm, pc + 4
*/
void bge(DECODER *decoder)
{
    if ((signed int)decoder->cpu->reg[decoder->rs1] >= (signed int)decoder->cpu->reg[decoder->rs2])
    {
        decoder->dnpc = decoder->cpu->PC + decoder->imm;
    }
}
/*
b INST ----BGEU
imm[12|10:5] rs2 rs1 111 imm[4:1|11] 1100011
pc = rs1 >= rs2 ? pc + imm, pc + 4
*/
void bgeu(DECODER *decoder)
{
    if (decoder->cpu->reg[decoder->rs1] >= decoder->cpu->reg[decoder->rs2])
    {
        decoder->dnpc = decoder->cpu->PC + decoder->imm;
        log_d("rs1>=rs2,reg[%d](%lx)>=reg[%d](%lx) ,next pc= %lx",decoder->rs1,decoder->cpu->reg[decoder->rs1],decoder->rs2,decoder->cpu->reg[decoder->rs2],decoder->dnpc);
        log_d("next pc = %lx,current pc = %lx,imm=%lx",decoder->dnpc,decoder->cpu->PC,decoder->imm);
    }else{
        log_d("rs1<rs2,next pc= %lx",decoder->dnpc+4);
    }
}

/*
B INST ---- BLT
imm[12|10:5] rs2 rs1 100 imm[4:1|11] 1100011
pc = s64'rs1 < s64'rs2 ? pc + imm, pc + 4
*/
void blt(DECODER *decoder)
{
    if ((signed int)decoder->cpu->reg[decoder->rs1] < (signed int)decoder->cpu->reg[decoder->rs2])
    {
        decoder->dnpc = decoder->cpu->PC + decoder->imm;
    }
}

/*
b inst  ----bltu
imm[12|10:5] rs2 rs1 110 imm[4:1|11] 1100011
pc = rs1 < rs2 ? pc + imm, pc + 4
*/
void bltu(DECODER *decoder)
{
    if (decoder->cpu->reg[decoder->rs1] < decoder->cpu->reg[decoder->rs2])
    {
        decoder->dnpc = decoder->cpu->PC + decoder->imm;
    }
}

/*
b inst -----bne
imm[12|10:5] rs2 rs1 001 imm[4:1|11] 1100011
pc <- rs1 != rs2 ? pc + imm, pc + 4
*/
void bne(DECODER *decoder)
{
    if (decoder->cpu->reg[decoder->rs1] != decoder->cpu->reg[decoder->rs2])
    {
        log_d("bne offset = 0x%lx, rs1 = 0x%lx, rs2 = 0x%lx\n", decoder->imm, decoder->cpu->reg[decoder->rs1], decoder->cpu->reg[decoder->rs2]);
        decoder->dnpc = decoder->cpu->PC + decoder->imm;
    }
}
/*
 I inst --- ebreak
 000000000001 00000 000 00000 1110011
*/
void ebreak(DECODER *decoder)
{
    // todo
    log_d("ebreak");
    exit(0);
    return;
}

/*
I inst --ecall
000000000000 00000 000 00000 1110011
*/
void ecall(DECODER *decoder)
{

    // decoder->rv_csr.mepc = decoder->snpc;
    decoder->cpu->csr.csr[mpec] = decoder->snpc;
    log_d("ecall,mepc = %lx", decoder->rv_csr.mepc);
    return;
}
/*
I inst --fence
000000000000 00000 001 00000 0001111
*/
void fence(DECODER *decoder)
{
    log_d("fence todo");
    return;
}

/*
I mret
000000000001 00000 000 00000 1110011 ebreak
000000000000 00000 000 00000 1110011 ecall
001100000010 00000 000 00000 1110011 mret

001100000010 00000 000 00000 1110011 mret

000
pc->next = CSRs[mepc],
*/
void mret(DECODER *decoder)
{
    log_d("mret,decoder->rv_csr.mepc=%lx", decoder->cpu->csr.csr[mpec]);
    decoder->snpc = decoder->cpu->csr.csr[mpec];
}

/*
J inst -- JAL
offset[20|10:1|11|19:12] rd 1101111
x[rd] = pc+4; pc += sext(offset)
*/
void jal(DECODER *decoder)
{
    
    decoder->cpu->reg[decoder->rd] = decoder->snpc; // pc+4
    decoder->dnpc = decoder->imm + decoder->cpu->PC;
    log_d("decoder->imm=%x,decoder->cpu->reg[%d]=%lx,next pc=%lx", decoder->imm,decoder->rd,decoder->cpu->reg[decoder->rd],decoder->dnpc);
}

/*
J inst --JALR
offset[11:0] rs1 010 rd 1100111
pc = (rs1 + imm) & ~1
rd = pc + 4
*/
void jalr(DECODER *decoder)
{
    decoder->dnpc = (decoder->imm + decoder->cpu->reg[decoder->rs1]) & -1;
    // if(decoder->dnpc <=0){
    //     decoder->dnpc = 0x80000000;
    // }
    decoder->cpu->reg[decoder->rd] = decoder->snpc; // pc+4

    log_d("jalr = %x,rs1=%d,next pc = %lx", decoder->imm,decoder->rs1,decoder->dnpc);
}

/*
I inst ---LB
imm[11:0] rs1 000 rd 0000011
x[rd] = sext(M[x[rs1] + sext(offset)][7:0])
*/
void lb(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = rv_signext(cpu_load(&cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 1), 7);
}

/*
I inst ----LBU
imm[11:0] rs1 100 rd 0000011
x[rd] = M[x[rs1] + sext(offset)][7:0]
*/
void lbu(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = cpu_load(&cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 1);
}

/*
I inst ----LH
imm[11:0] rs1 001 rd 0000011
x[rd] = sext(M[x[rs1] + sext(offset)][15:0])
*/
void lh(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = rv_signext(cpu_load(&cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 2), 15);
}

/*
I inst ----LHU
offset[11:0] rs1 101 rd 0000011
x[rd] = M[x[rs1] + sext(offset)][15:0]
*/
void lhu(DECODER *decoder)
{
    cpu_load(&cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 2);
}

/*
I inst ----LUI
immediate[31:12] rd 0110111
x[rd] = sext(immediate[31:12] << 12)
*/
void lui(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->imm << 12;
}

/*
U inst --- LW
imm[11:0] rs1 010 rd 0000011
x[rd] = sext(M[x[rs1] + sext(offset)][31:0])
*/
void lw(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = rv_signext(cpu_load(&cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 4), 31);
}

/*
R inst --- OR
0000000 rs2 rs1 110 rd 0110011
x[rd] = x[rs1] | 𝑥[𝑟𝑠2]
*/
void or (DECODER * decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] | decoder->cpu->reg[decoder->rs2];
}

/*
I inst ---ORI
Immediate[11:0] rs2 rs1 110 rd 0010011
x[rd] = x[rs1] | sext(immediate)
*/
void ori(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] | decoder->imm;
}

/*
S inst ---SB
imm[11:5] rs2 rs1 000 imm[4:0] 0100011
M[x[rs1] + sext(offset) = x[rs2][7: 0]
*/
void sb(DECODER *decoder)
{
    log_d("sb=%x", decoder->cpu->reg[decoder->rs1] + decoder->imm);
    cpu_store(decoder->cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 1, decoder->cpu->reg[decoder->rs2]);
}

/*
S inst ---SH
imm[11:5] rs2 rs1 001 imm[4:0] 0100011
M[x[rs1] + sext(offset) = x[rs2][15: 0]
*/
void sh(DECODER *decoder)
{
    cpu_store(decoder->cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 2, decoder->cpu->reg[decoder->rs2]);
}

/*
R inst ---SLL
0000000 rs2 rs1 001 rd 0110011
x[rd] = x[rs1] ≪ x[rs2]
*/
void sll(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] << rv_bf(decoder->cpu->reg[decoder->rs2], 4, 0);
}

/*
R inst ---SLT
0000000 rs2 rs1 010 rd 0110011
x[rd] = (x[rs1] <𝑠 x[rs2])
*/
void slt(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = (long)decoder->cpu->reg[decoder->rs1] < (long)decoder->cpu->reg[decoder->rs2];
}

/*
I inst ---SLTI
immediate[11:0] rs1 010 rd 0010011
x[rd] = (x[rs1] <𝑠 sext(immediate))
*/
void slti(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = (long)decoder->cpu->reg[decoder->rs1] < (long)decoder->imm ? 1 : 0;
}

/*
I inst ---SLTIU
immediate[11:0] rs1 011 rd 0010011
x[rd] = (x[rs1] <𝑢 sext(immediate))
*/
void sltiu(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] < decoder->imm ? 1 : 0;
}

/*
R inst ---SLTU
immediate[11:0] rs1 011 rd 0010011
x[rd] = (x[rs1] <𝑢 sext(immediate))
*/
void sltu(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] < decoder->cpu->reg[decoder->rs2] ? 1 : 0;
}

/*
R inst ---SRA
0100000 rs2 rs1 101 rd 0110011
x[rd] = (x[rs1] ≫𝑠 x[rs2])
*/
void sra(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = (long)decoder->cpu->reg[decoder->rs1] >> rv_bf(decoder->cpu->reg[decoder->rs2], 4, 0);
}

/*
R inst ---SRL
0000000 rs2 101 rd 0110011
x[rd] = (x[rs1] <<𝑠 x[rs2])
*/
void srl(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] << rv_bf(decoder->cpu->reg[decoder->rs2], 4, 0);
}

/*
R inst ---SUB
0100000 rs2 000 rd 0110011
x[rd] = x[rs1] − x[rs2]
*/
void sub(DECODER *decoder)
{
    log_d("sub");
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] - decoder->cpu->reg[decoder->rs2];
}

/*
S inst ---SW
imm[11:5] rs2 010 imm[4:0] 0100011
M[x[rs1] + sext(offset) = x[rs2][31: 0]
*/
void sw(DECODER *decoder)
{
    log_d("decoder->cpu->reg[%d]=%lx,decoder->cpu->reg[%d] = %lx,imm = %lx", decoder->rs1, decoder->cpu->reg[decoder->rs1], decoder->rs2,decoder->cpu->reg[decoder->rs2],decoder->imm);
    cpu_store(decoder->cpu, decoder->cpu->reg[decoder->rs1] + decoder->imm, 4, decoder->cpu->reg[decoder->rs2]);
}

/*
R inst ---XOR
0000000 rs2 100 rd 0110011
x[rd] = x[rs1] ^ x[rs2]
*/
void xor (DECODER * decoder) {
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] ^ decoder->cpu->reg[decoder->rs2];
}

    /*
    I inst ---XORI
    imm[11:0] 100 rd 0010011
    x[rd] = x[rs1] ^ sext(immediate)
    */
    void xori(DECODER *decoder)
{
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] ^ decoder->imm;
}
/*
I inst ---SLRI
000000 shamt rs1 101 rd 0010011
x[rd] = (x[rs1] ≫𝑢 shamt)
*/
void srli(DECODER *decoder)
{
    log_d("slri --- decoder->rd = %d,decoder->rd=%d", decoder->rd, decoder->rs1);
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] >> decoder->shamt;
}
/*
I inst ---SRAI
0100000 shamt rs1 101 rd 0010011
x[rd] = x[rs1] ≫ shamt
*/
void srai(DECODER *decoder)
{
    log_d("srai --- decoder->rd = %d,decoder->rd=%d", decoder->rd, decoder->rs1);
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] >> decoder->shamt;
}
/*
I csrrw
csr 00000 001 rd 1110011
t = CSRs[csr]; CSRs[csr] = x[rs1]; x[rd] = t
*/
void csrrw(DECODER *decoder)
{
    // uint32_t val;
    // if (decoder->rd != 0)
    // {
    //     val = decoder->cpu->csr.csr[decoder->csr_addr];
    // }
    // else
    // {
    //     val = 0;
    // }
    uint32_t val = decoder->rd == 0 ? 0 : decoder->cpu->csr.csr[decoder->csr_addr];
    decoder->cpu->csr.csr[decoder->csr_addr] = decoder->cpu->reg[decoder->csr_addr];
    decoder->cpu->reg[decoder->rd] = val;
    log_d("val = 0x%08lx, rs1val = 0x%08lx\r\n", val, decoder->cpu->csr.csr[decoder->csr_addr]);
}

/*
I csrrs
csr rs1 010 rd 1110011
t = CSRs[csr]; CSRs[csr] = t | x[rs1]; x[rd] = t
*/
void csrrs(DECODER *decoder)
{
    uint32_t val = decoder->cpu->csr.csr[decoder->csr_addr];
    uint32_t rs1_val = decoder->rs1 == 0 ? 0 : decoder->cpu->reg[decoder->rs1];
    log_d("before val = 0x%08lx, rs1_val = 0x%08lx", val, rs1_val);
    decoder->cpu->reg[decoder->rd] = val;
    if (decoder->rs1 != 0)
    {
        decoder->cpu->csr.csr[decoder->csr_addr] = val | rs1_val;
    }
}

/*
I csrrc
csr rs1 011 rd 1110011
t = CSRs[csr]; CSRs[csr] = t &~x[rs1]; x[rd] = t
*/
void csrrc(DECODER *decoder)
{
    uint32_t val = decoder->cpu->csr.csr[decoder->csr_addr];
    uint32_t rs1_val = decoder->rs1 == 0 ? 0 : decoder->cpu->reg[decoder->rs1];
    decoder->cpu->reg[decoder->rd] = val;
    rs1_val = ~rs1_val;
    if (decoder->rs1 != 0)
    {
        decoder->cpu->csr.csr[decoder->csr_addr] = val & rs1_val;
    }
}

/*
I csrrwi
csr rs1 101 rd 1110011
x[rd] = CSRs[csr]; CSRs[csr] = zimm
*/
void csrrwi(DECODER *decoder)
{
    uint32_t zimm = decoder->rs1;
    uint32_t val = decoder->rd == 0 ? 0 : decoder->cpu->csr.csr[decoder->csr_addr];
    decoder->cpu->reg[decoder->rd] = val;
    decoder->cpu->csr.csr[decoder->csr_addr] = zimm;
}

/*
I csrrsi
csr imm 010 rd 1110011
t = CSRs[csr]; CSRs[csr] = t | zimm; x[rd] = t
*/
void csrrsi(DECODER *decoder)
{
    uint32_t zimm = decoder->rs1;
    uint32_t val = decoder->cpu->csr.csr[decoder->csr_addr];
    decoder->cpu->reg[decoder->rd] = val;
    decoder->cpu->csr.csr[decoder->csr_addr] = val | zimm;
}

/*
I csrrci
csr imm 010 rd 1110011
t = CSRs[csr]; CSRs[csr] = t | zimm; x[rd] = t
*/
void csrrci(DECODER *decoder)
{
    uint32_t zimm = decoder->rs1;
    uint32_t val = decoder->cpu->csr.csr[decoder->csr_addr];
    decoder->cpu->reg[decoder->rd] = val;
    zimm = ~zimm;
    decoder->cpu->csr.csr[decoder->csr_addr] = val & zimm;
}

/*
I slli
0000000 shamt 001 rd 0010011
x[rd] = x[rd] << uimm
*/
void slli(DECODER *decoder)
{
    log_d("slli");
    //    log_d("decoder->rs1:0x%08x,decoder->shamt=0x%08x",decoder->cpu->reg[decoder->rs1],decoder->shamt);
    decoder->cpu->reg[decoder->rd] = decoder->cpu->reg[decoder->rs1] << decoder->shamt;
}

void init_inst_funct()
{
    set_inst_func(ADD, add);
    set_inst_func(ADDI, addi);
    set_inst_func(AND, and);
    set_inst_func(ANDI, andi);
    set_inst_func(AUIPC, auipc);
    set_inst_func(BEQ, beq);
    set_inst_func(BGE, bge);
    set_inst_func(BGEU, bgeu);
    set_inst_func(BLT, blt);
    set_inst_func(BLTU, bltu);
    set_inst_func(BNE, bne);
    set_inst_func(EBRERAK, ebreak);
    set_inst_func(ECALL, ecall);
    set_inst_func(FENCE, fence);
    set_inst_func(MRET, mret);
    set_inst_func(JAL, jal);
    set_inst_func(JALR, jalr);
    set_inst_func(LB, lb);
    set_inst_func(SUB, sub);
    set_inst_func(LBU, lbu);
    set_inst_func(LH, lh);
    set_inst_func(LHU, lhu);
    set_inst_func(LUI, lui);
    set_inst_func(LW, lw);
    set_inst_func(OR, or);
    set_inst_func(ORI, ori);
    set_inst_func(XORI, xori);
    set_inst_func(SLLI, slli);
    set_inst_func(SH, sh);
    set_inst_func(SB, sb);
    set_inst_func(SLL, sll);
    set_inst_func(SLT, slt);

    set_inst_func(SLTI, slti);
    set_inst_func(SLTIU, sltiu);
    set_inst_func(SB, sb);
    set_inst_func(SLTU, sltu);
    set_inst_func(SRA, sra);

    set_inst_func(SRL, srl);
    set_inst_func(SRLI, srli);
    set_inst_func(SRAI, srai);
    set_inst_func(SW, sw);
    set_inst_func(XOR, xor);
    // set_inst_func(XORI, xori);
    // ZICSR
    set_inst_func(CSRRW, csrrw);
    set_inst_func(CSRRS, csrrs);
    set_inst_func(CSRRC, csrrc);
    set_inst_func(CSRRWI, csrrwi);
    set_inst_func(CSRRSI, csrrsi);
    set_inst_func(CSRRCI, csrrci);
}

uint32_t inst_fetch(CPU *cpu)
{
    return cpu_load(cpu, cpu->PC, 4);
}

void exec_once(CPU *cpu)
{
    cpu->reg[0] = 0;
    uint32_t inst = inst_fetch(cpu); // get inst
    if (0xc0001073 == inst)
    {
        cpu->cpu_state = CPU_STOP;
        log_d("pass isa test");
        return;
    }
    
    DECODER decoder = decode(inst);
    if (decoder.inst_name == INST_NUM)
    {
        cpu->cpu_state = CPU_STOP;
        log_e("Unsupported instruction or EOF");
        log_e("PC = 0x%08lx inst=0x%08x inst_name = %d\n", cpu->PC, inst, decoder.inst_name);
        print_regs();
        return;
    }
    log_i("PC=%lx,inst = 0x%08x,inst num = %d ", decoder.dnpc,inst,decoder.inst_name);
    // decoder.dnpc = decoder.snpc = cpu->PC + 4;
    decoder.dnpc = decoder.snpc = cpu->PC+4;
    decoder.cpu = cpu;
    
    inst_handler[decoder.inst_name](&decoder);
#ifdef DUG_REG
    printf_regs();
#endif
    cpu->PC = decoder.dnpc;
}

int cpu_exec(CPU *cpu, uint32_t n)
{
    for (int i = 0; i < n; i++)
    {
        if (cpu->cpu_state == CPU_RUN)
        {
            exec_once(cpu);
        }
        else if (cpu->cpu_state == CPU_STOP)
        {
            log_d("exit!!!!!!!");
            return (cpu->reg[17] == 93 && (cpu->reg[10] == 0) && (cpu->reg[3] == 1) ? EXIT_SUCCESS : EXIT_FAILURE);
        }
        // return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
void printf_regs()
{
    for (int i = 0; i < 32; i++)
    {
        if (cpu.reg[i] >= DRAM_BASE)
        {
            log_e("reg[%d]:%ld", i, cpu.reg[i] - DRAM_BASE);
        }
        else
        {
            log_e("reg[%d]:%ld", i, cpu.reg[i]);
        }
    }
}