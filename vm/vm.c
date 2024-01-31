#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_SIZE_UINT16 65535
#define FUNC_COUNT 16

#define GET_OPC(opc) ((opc) >> 12)
#define GET_OFFSET_ADDR(val) transform_imm_to_uint16((val)&0x1ff, 9)
#define GET_FIVE_RIGHT_BITS(val) ((val >> 5)&1)
#define GET_VAL(val) ((val)&0x1f)
#define GET_DR_ADDR(val) ((val >> 9)&0x7)
#define GET_SR1_ADDR(val) ((val >> 6)&0x7)
#define GET_SR2_ADDR(val) ((val)&0x7)
#define GET_TRAP_OPC(val) ((val) & 0xff)

uint16_t MEM_START_ADDR = 0x3000;
uint16_t mem[MAX_SIZE_UINT16] = {0};

/*
R0 - stdin\stdout
R1-R7 - рег. о.н.
RPC - адр. след. инстр.
RCND - флаги
*/

enum stat {running, disabled};
enum stat status = running;

enum registers {R0 = 0, R1, R2, R3, R4, R5, R6, R7, RPC, RCND, RCNT};
uint16_t registers[RCND] = {0};
enum flags {FP = 1, FZ = 2, FN = 4};

static uint16_t mread(uint16_t addr) {return mem[addr];}
static uint16_t mwrite(uint16_t addr, uint16_t val) {mem[addr] = val;}

static void set_flags(enum registers rgstr){
    if (registers[rgstr] == 0) registers[RCND] = FZ;
    else if (registers[rgstr] >> 15) registers[RCND] = FN; 
    else registers[RCND] = FP;
}


static uint16_t sext(uint16_t n, int b) { 
    return ((n>>(b-1))&1) ? (n|(0xFFFF << b)) : n;
}
#define SEXTIMM(i) sext(GET_VAL(i),5)

static void add(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] + SEXTIMM(val);
    else registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] + registers[GET_SR2_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void and(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] & SEXTIMM(val);
    else registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] & registers[GET_SR2_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void or(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] | SEXTIMM(val);
    else registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] | registers[GET_SR2_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void not(uint16_t val){
    registers[GET_FIVE_RIGHT_BITS(val)] =~ registers[GET_SR1_ADDR(val)];
    set_flags(GET_DR_ADDR(val));
}

static void xor(uint16_t val){
    if (GET_FIVE_RIGHT_BITS(val)) registers[GET_DR_ADDR(val)] = registers[GET_SR1_ADDR(val)] ^ SEXTIMM(val);
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

typedef void (*trap_exec_func)();
enum {trap_offset = 0x20};

trap_exec_func trap_exec[6] = {tgetc, tputc, tputs, tgetu16, tputu16, tend};

static void trap(uint16_t val){
    trap_exec[GET_TRAP_OPC(val) - trap_offset]();
}


enum{
    OPC_ADD = 1,
    OPC_AND,
    OPC_OR,
    OPC_NOT,
    OPC_XOR,
    OPC_TRAP = 15
};

int load_img(char *filename, uint16_t offset){
    puts("load img");
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
    puts("start");
    registers[RPC] = MEM_START_ADDR + offset;
    while(status == running){
        uint16_t instr = mread(registers[RPC]++);
        // printf("\ninstr %x\n", instr);
        switch(GET_OPC(instr)){
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
                printf("undefined instruction %i\n", GET_TRAP_OPC(instr));
                break;
                // exit(EXIT_SUCCESS);
        }
    }
}

// uint16_t program[] = {
//                         0xF023,    
//                         0xF024,
//                         0x1220,
//                         0xF023,
//                         0x1240,
//                         0x1060,
//                         0xF024,
//                         0xF025
// };

uint16_t program[] = {
    /*mem[0x3000]=*/    0xF023,    //  1111 0000 0010 0011             TRAP trp_in_u16  
    /*mem[0x3002]=*/    0x1220,    //  0001 0010 0010 0000             ADD R1,R0,x0     
    /*mem[0x3003]=*/    0xF023,    //  1111 0000 0010 0011             TRAP trp_in_u16  
    /*mem[0x3004]=*/    0x1240,    //  0001 0100 0010 0000             ADD R1,R1,R0    
    /*mem[0x3006]=*/    0x1060,    //  0001 0000 0110 0000             ADD R0,R1,x0     
    /*mem[0x3007]=*/    0xF024,    //  1111 0000 0010 0100             TRAP trp_out_u16
    /*mem[0x3006]=*/    0xF025,    //  1111 0000 0010 0101             HALT          
};

int gen_obj_file() {
    char *outf = "test.obj";
    FILE *f = fopen(outf, "wb");
    if (NULL==f) {
        perror(outf);
        exit(EXIT_FAILURE);
    }
    size_t writ = fwrite(program, sizeof(uint16_t), sizeof(program), f);
    printf("Written size_t = %lu to file %s\n", writ, outf);
    fclose(f);
    return 0;
}

int main(int argc, char **argv){
    if(gen_obj_file() != 0) perror("Error generate obj file\n");
    if (load_img(argv[1], 0) != 0){
        printf("Failed load program\n");
        exit(EXIT_FAILURE);
    }
    start(0);
    return 0;
}

