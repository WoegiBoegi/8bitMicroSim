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
 * 6x H || L
 * A x0 || x8
 * B x1 || x9
 * C x2 || xa
 * D x3 || xb
 * E x4 || xc
 * F x5 || xd
 * H x6 || xe
 * M x7 || xf
 *
 * 0x30 - 0x6f reserved for MOV operations
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
    printf("value in Accumulator: %02x\nvalue in Register B:  %02x\n"
           "value in Register C:  %02x\nvalue in Register D:  %02x\nvalue in Register E:  %02x\n"
           "value in Register F:  %02x\nvalue in Register H:  %02x\nvalue in Register L:  %02x\n",
           ACC,REG_B,REG_C,REG_D,REG_E,REG_F,REG_H,REG_L);
    printf("dumping Memory:\n");
    printMEM();
    printf("========================================================\n");
}

int MOV(char OP){
    char digitOne = (OP/0x10)*0x10;
    char digitTwo = OP-(digitOne);
    int isTwoHigh = 0;
    if(digitTwo>0x07){
        isTwoHigh = 1;
    }

    unsigned char *REGOne;
    unsigned char *REGTwo;
    switch(digitOne){
        case 0x30:
            if(isTwoHigh){
                REGOne = &REG_B;
            }
            else{
                REGOne = &ACC;
            }
            break;
        case 0x40:
            if(isTwoHigh){
                REGOne = &REG_D;
            }
            else{
                REGOne = &REG_C;
            }
        case 0x50:
            if(isTwoHigh){
                REGOne = &REG_F;
            }
            else{
                REGOne = &REG_E;
            }
        case 0x60:
            if(isTwoHigh){
                REGOne = &REG_L;
            }
            else{
                REGOne = &REG_H;
            }
    }

    switch(digitTwo){
        case 0x00:
        case 0x08:
            REGTwo = &ACC;
            break;
        case 0x01:
        case 0x09:
            REGTwo = &REG_B;
            break;
        case 0x02:
        case 0x0a:
            REGTwo = &REG_C;
            break;
        case 0x03:
        case 0x0b:
            REGTwo = &REG_D;
            break;
        case 0x04:
        case 0x0c:
            REGTwo = &REG_E;
            break;
        case 0x05:
        case 0x0d:
            REGTwo = &REG_F;
            break;
        case 0x06:
        case 0x0e:
            REGTwo = &REG_H;
            break;
        case 0x07:
        case 0x0f:
            REGTwo = &REG_L;
            break;
    }

    *REGOne = *REGTwo;

    return 0;
}

int getInstruction(char OP){
    if(OP >= 0x30 && OP <= 0x7f){
        return MOV(OP);
    }
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
                ACC = RAM[ProgramCounter+1];
                ProgramCounter++;
                break;
            case STA:
                RAM[RAM[ProgramCounter+1]] = ACC;
                ProgramCounter++;
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
                if(getInstruction(OP)){
                    printf("ERROR: invalid opcode detected, cannot execute");
                    printDebugInfo();
                    return 1;
                }
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
