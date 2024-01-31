#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_SIZE_UINT16 65535
#define FUNC_COUNT 16

#define GET_OPC(opc) ((opc) >> 12) //получение опкода (первых 4 бит)
#define IMM5_TO_UINT16(val) transform_imm_to_uint16(val, 5)
#define GET_OFFSET_ADDR(val) transform_imm_to_uint16((val)&0x1ff, 9)
#define GET_FIVE_RIGHT_BITS(val) ((val >> 5)&1)
#define GET_VAL(val) ((val)&0x1f)
#define GET_DR_ADDR(val) ((val >> 9)&0x7)
#define GET_SR1_ADDR(val) ((val >> 6)&0x7)
#define GET_SR2_ADDR(val) ((val)&0x7)
#define GET_TRAP_OPC(val) ((val)&0xff)

uint16_t MEM_START_ADDR = 0x3000;
uint16_t mem[MAX_SIZE_UINT16] = {0};

/*
R0 - чтение\запись в stdin\stdout
R1-R7 - просто регистры общего назначения
RPC - адрес следующей инструкции
RCND - регистр флагов
*/

enum stat {running, disabled};
enum stat status = running;

enum registers {R0 = 0, R1, R2, R3, R4, R5, R6, R7, RPC, RCND, RCNT};
uint16_t registers[RCND] = {0};
enum flags {FP = 1, FZ = 2, FN = 4};

typedef void (*trap_exec_func)();
enum {trap_offset = 0x20};

static uint16_t mread(uint16_t addr) {return mem[addr];}
static uint16_t mwrite(uint16_t addr, uint16_t val) {mem[addr] = val;}

static void set_flags(enum registers rgstr){
    if (registers[rgstr] == 0) registers[RCND] = FZ;
    else if (registers[rgstr] >> 15) registers[RCND] = FN; // >>15 получение первого бита, отвечающего за знак
    else registers[RCND] = FP;
}

static inline uint16_t transform_imm_to_uint16(uint16_t n, int b){
    if ((n >> (b-1)) &1) return (n|0xFFFF << b);
    else return n;
}

static void add(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] + IMM5_TO_UINT16(val);
    else registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] + registers[GET_SR2_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void and(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] & IMM5_TO_UINT16(val);
    else registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] & registers[GET_SR2_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void or(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] | IMM5_TO_UINT16(val);
    else registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] | registers[GET_SR2_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void not(uint16_t val){
    registers[GET_FIVE_RIGHT_BITS(val)] =~ registers[GET_SR1_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void xor(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] ^ IMM5_TO_UINT16(val);
    else registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] ^ registers[GET_SR2_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void tgetc(){
    registers[R0] = getchar();
}
static void tputc(){
    fprintf(stdout, "%c", (char)registers[R0]);
}
static void tputs(){
    uint16_t *p = mem + registers[R0];
    while(*p){
        fprintf(stdout, "%c", (char)*p);
        p++;
    }
}
static void tgetu16(){
    fscanf(stdin, "%hu", &registers[R0]);
}
static void tputu16(){
    fprintf(stdout, "%hu", registers[R0]);
}
static void tend(){
    status = disabled;
}

trap_exec_func trap_exec[6] = {tgetc, tputc, tputs, tgetu16, tputu16, tend};
static void trap(uint16_t val){
    trap_exec[GET_TRAP_OPC(val) - trap_offset]();
}


enum{
    OPC_ADD = 0,
    OPC_AND,
    OPC_OR,
    OPC_NOT,
    OPC_XOR,
    OPC_TRAP
};

int load_img(char *filename, uint16_t offset){
    FILE *input_file = fopen(filename, "rb");

    if (input_file == NULL){
        perror(input_file);
        fclose(input_file);
        exit(EXIT_FAILURE);
    }

    uint16_t *position_for_read = mem + MEM_START_ADDR + offset;
    fread(position_for_read, sizeof(uint16_t), (MAX_SIZE_UINT16 - MEM_START_ADDR), input_file); 
    fclose(input_file);
    return 0;
}

void start(uint16_t offset){
    registers[RPC] = MEM_START_ADDR + offset;
    while(status == running){
        uint16_t instr = mread(registers[RPC]++);

        switch(instr & 0xff){
            case OPC_ADD:
            {
                add(instr);
                break;
            }

            case OPC_AND:
            {
                and(instr);
                break;
            }
            case OPC_OR:
            {
                or(instr);
                break;
            }
            case OPC_NOT:
            {
                not(instr);
                break;
            }
            case OPC_XOR:
            {
                xor(instr);
                break;
            }
            case OPC_TRAP:
            {
                trap(instr);
                break;
            }
            default: 
                break;
        }
    }
}

uint16_t program[] = {
    /*mem[0x3000]=*/    0xF026,    //  1111 0000 0010 0110             TRAP trp_in_u16  ;считывает uint16_t из stdin и помещает его в R0
    /*mem[0x3002]=*/    0x1220,    //  0001 0010 0010 0000             ADD R1,R0,x0     ;прибавляет содержимое R0 к R1
    /*mem[0x3003]=*/    0xF026,    //  1111 0000 0010 0110             TRAP trp_in_u16  ;считывает uint16_t из stdin и помещает его в R0
    /*mem[0x3004]=*/    0x1240,    //  0001 0010 0010 0000             ADD R1,R1,R0     ;прибавляет содержимое R0 к R1
    /*mem[0x3006]=*/    0x1060,    //  0001 0000 0110 0000             ADD R0,R1,x0     ;прибавляет содержимое R1 к R0
    /*mem[0x3007]=*/    0xF027,    //  1111 0000 0010 0111             TRAP trp_out_u16;выводит содержимое R0 в stdout
    /*mem[0x3006]=*/    0xF025,    //  1111 0000 0010 0101             HALT             ;остановка
};

int gen_obj_file() {
    char *outf = "sum.obj";
    FILE *f = fopen(outf, "wb");
    if (NULL==f) {
        fprintf(stderr, "Cannot write to file %s\n", outf);
    }
    size_t writ = fwrite(program, sizeof(uint16_t), sizeof(program), f);
    fprintf(stdout, "Written size_t=%lu to file %s\n", writ, outf);
    fclose(f);
    return 0;
}

int main(int argc, char **argv){
    gen_obj_file();
    if (load_img(argv[1], 0) != 0){
        fprintf(stdout, "Failed load program\n");
        exit(EXIT_FAILURE);
    }
    start(0);
    return 0;
}

