#include <stdint.h>

#define MAX_SIZE_UINT16 65535
#define FUNC_COUNT 16

#define GET_OPC(opc) ((opc) >> 12) //получение опкода (первых 4 бит)
#define IMM5_TO_UINT16(val) transform_imm_to_uint16(val, 5)
#define GET5BIT(val) ((val >> 5)&1)

uint16_t MEM_START_ADDR = 0x3000;
uint16_t mem[MAX_SIZE_UINT16] = {0};

/*
R0 - чтение\запись в stdin\stdout
R1-R7 - просто регистры общего назначения
RPC - адрес следующей инструкции
RCND - регистр флагов
*/

enum registers {R0 = 0, R1, R2, R3, R4, R5, R5, R6, R7, RPC, RCND, RCNT};
uint16_t reg[RCND] = {0};
enum flags {FP = 1 << 0, FZ = 1 << 1, FN = 1 << 2};

static uint16_t mread(uint16_t addr) {return mem[addr];}
static uint16_t mwrite(uint16_t addr, uint16_t val) {mem[addr] = val;}

static inline void set_flags(enum registers rgstr){
    if (reg[rgstr] == 0) reg[RCND] = FZ;
    else if (reg[rgstr] >> 15) reg[RCND] = FN; // >>15 получение первого бита, отвечающего за знак
    else reg[RCND] = FP;
}

static inline uint16_t transform_imm_to_uint16(uint16_t n, int b){
    if ((n >> (b-1)) &1) return (n|0xFFFF << b);
    else return n;
}


typedef void (*op_ex_func)(uint16_t instruction);

static inline void add(uint16_t val){
    if (GET5BIT(val)) reg[DR(val)] = reg[SR1(val)] + IMM5_TO_UINT16(val);
    else reg[DR(val)] = reg[SR1(val)] + reg[SR2(val)];
    set_flags(DR(val));
}

op_ex_func op_ex[FUNC_COUNT] = {br, add, ld, st, jsr, };
