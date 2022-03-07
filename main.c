#include <stdio.h>

#define MEMSIZE 0x100
#define MEMFILEPATH "/home/woegi/OneDrive/Projects/CPUSim/mem.bin" //"C:\\Users\\woegi\\OneDrive\\Projects\\CPUSim\\mem.bin"

#define NOP 0x00    //no operation, do nothing
#define ADDA 0x80   //Add value of ACC to ACC
#define ADDB 0x81   //Add value of REG_B to ACC //B = Register for Teletype Interrupt
#define ADDC 0x82   //Add value of REG_C to ACC
#define ADDD 0x83   //Add value of REG_D to ACC
#define ADDE 0x84   //Add value of REG_E to ACC
#define ADDF 0x85   //Add value of REG_F to ACC //F = Flag Register
#define ADDH 0x87   //Add value of REG_H to ACC //H = Memory Address Register High 0xXX00
#define ADDM 0x87   //Add value of REG_L to ACC //L = Memory Address Register Low  0x00XX
#define SUBA 0x8a   //Subtract value of ACC from ACC
#define SUBB 0x8a   //Subtract value of REG_B from ACC
#define SUBC 0x8a   //Subtract value of REG_C from ACC
#define SUBD 0x8a   //Subtract value of REG_D from ACC
#define SUBE 0x8a   //Subtract value of REG_E from ACC
#define SUBF 0x8a   //Subtract value of REG_F from ACC
#define SUBH 0x8a   //Subtract value of REG_H from ACC
#define SUBL 0x8a   //Subtract value of REG_L from ACC
#define HLT 0x0F    //halt execution
#define LDA 0x2A    //load value (in next Memory address) into Accumulator
#define STA 0x2B    //store value from accumulator in memory address (specified in the next memory address)
#define LDAM 0x21   //load value (from Memory address specified in REG_H and REG_L) into ACC
#define STAM 0x22   //store value from ACC in Memory address (specified in REG_H and REG_L)

/*
 * MOV INSTRUCTION
 * 3x A || B
 * 4x C || D
 * 5x E || F
 * 7x H || M
 * A x0 || x8
 * B x1 || x9
 * C x2 || xa
 * D x3 || xb
 * E x4 || xc
 * F x5 || xd
 * H x6 || xe
 * M x7 || xf
 *
 * 0x30 - 0x7f reserved for MOV operations
 * eg. 0x5c = MOV F,E
 */


unsigned int ProgramCounter = 0;
unsigned char ACC = 0;
unsigned char REG_B = 0;    //used by Teletype interrupt to print char to screen buffer
unsigned char REG_C = 0;
unsigned char REG_D = 0;
unsigned char REG_E = 0;
unsigned char REG_F = 0;    //Flag Register
unsigned char REG_H = 0;    //Memory Register High XX00
unsigned char REG_L = 0;    //Memory Register Low  00XX

//TODO: Add Call and Return, JMP, JNE, JNZ, JZ, etc.

unsigned char RAM[MEMSIZE];

void printMEM(){
    printf("--------------------------------------------------------\n");
    printf("      00 01 02 03  04 05 06 07  08 09 0a 0b  0c 0d 0e 0f\n");

    for(int ADDR = 0x00; ADDR <= (MEMSIZE-0x10); ADDR += 0x10){
        printf("%04x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x\n", ADDR, RAM[ADDR+0x00], RAM[ADDR+0x01], RAM[ADDR+0x02], RAM[ADDR+0x03], RAM[ADDR+0x04], RAM[ADDR+0x05], RAM[ADDR+0x06], RAM[ADDR+0x07], RAM[ADDR+0x08], RAM[ADDR+0x09], RAM[ADDR+0x0a], RAM[ADDR+0x0b], RAM[ADDR+0x0c], RAM[ADDR+0x0d], RAM[ADDR+0x0e], RAM[ADDR+0x0f]);
    }
    printf("--------------------------------------------------------\n");
}

void printDebugInfo(){
    printf("\n========================================================\n");
    printf("EXECUTION HALTED: DUMPING ALL INFO:\n");
    //print all registers, flags, etc. as well
    printf("program counter at position %04x\n",ProgramCounter);
    printf("value in Accumulator: %02x\nvalue in Register B: %02x\n",ACC,REG_B);
    printf("dumping Memory:\n");
    printMEM();
    printf("========================================================\n");
}

int MOV(char OP){
    if(OP >= 0x30 && OP <= 0x7f)
    return 1;
}

int MainLoop(){
    while(1){
        unsigned char OP = RAM[ProgramCounter];
        unsigned int HLADDR = REG_H*0x100+REG_L;
        unsigned int MEMADDR = RAM[ProgramCounter+1]*0x100+RAM[ProgramCounter+2];
        switch(OP){
            case NOP:
                break;
            case HLT:
                printDebugInfo();
                return 0;
            case LDA:
                ACC = RAM[MEMADDR];
                ProgramCounter+=2;
                break;
            case STA:
                RAM[RAM[MEMADDR]] = ACC;
                ProgramCounter+=2;
                break;
            case LDAM:
                ACC = RAM[HLADDR];
                break;
            case STAM:
                RAM[HLADDR] = ACC;
                break;
            case ADDB:
                ACC += REG_B;
                break;
            default:
                int ret = get
                printf("ERROR: invalid opcode detected, cannot execute");
                printDebugInfo();
                return 1;
        }
        ProgramCounter++; //advance Program Counter by one, execute next instruction on next loop
    }
}

int main() {
    FILE *src;

    src = fopen(MEMFILEPATH, "rb");  // r for read, b for binary
    if(src == NULL){
        printf("FILE READ ERROR\n");
        return 1;
    }
    fread(RAM, sizeof(RAM), 1, src);

    int ret = MainLoop();

    if(ret != 0){
        printf("PROGRAM HAS ENCOUNTERED AN ERROR, SEE INFODUMP FOR DETAILS\n");
    }
    else{
        printf("Program has executed without errors, dumped info for debug reasons :)\n");
    }
    return 0;
}
