#include <stdio.h>
#include <stdbool.h>

#define MEMSIZE 0x100
#define MEMFILEPATH "/home/woegi/OneDrive/Projects/CPUSim/mem.bin" //"C:\\Users\\woegi\\OneDrive\\Projects\\CPUSim\\mem.bin"

#define NOP 0x00    //no operation, do nothing
#define HLT 0x0F    //halt execution
#define LDA 0x2A    //load value into accumulator from memory address (specified in the next two memory addresses)
#define LDAL 0x2B   //load value (in next Memory address) into Accumulator
#define STA 0x2C    //store value from accumulator in memory address (specified in the next two memory addresses)
#define LDAM 0x21   //load value (from Memory address specified in REG_H and REG_L) into ACC
#define STAM 0x22   //store value from ACC in Memory address (specified in REG_H and REG_L)
#define JMP  0x60
#define JE   0x61   //jumps to address if zero flag set
#define JNE  0x62   //jumps to address if zero flag not set
#define JLT  0x6a   //jumps to address if zero flag not set AND sign flag not set
#define JGT  0x6f   //jumps to address if zero flag not set AND sign flag set


/*
 * ADD AND SUB INSTRUCTION
 * 8x ADD || SUB
 *
 * A  x0  || x8
 * B  x1  || x9
 * C  x2  || xa
 * D  x3  || xb
 * E  x4  || xc
 * F  x5  || xd
 * H  x6  || xe
 * L  x7  || xf
 *
 * 0x80 - 0x8f reserved for ADD and SUB operations
 * e.g. 0x8c = SUB A,E = subtract the value in E from the value in A
*/


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
 * e.g. 0x5c = MOV F,E = copy the value of E into F
 */



unsigned int ProgramCounter = 0;
unsigned char ACC = 0;
unsigned char REG_B = 0;    //used by Teletype interrupt to print char to screen buffer
unsigned char REG_C = 0;
unsigned char REG_D = 0;
unsigned char REG_E = 0;

/*Flag Register
*  7 6 5 4 3 2 1 0
*  S Z 0 0 0 P 0 C
*  S: Sign Flag, 1 if result of previous operation is positive
*  Z: Zero Flag, 1 of result of previous operation is zero
*  P: Parity indicates amount of 1s in operation in binary, 1 if result has even number of 1s
*  C: Carry Flag, if carry was perfored in most significant bit i.e. 254+6 = 260 > 255 => C = 1
*  0: unused, always zero
*/
unsigned char REG_F = 0;
unsigned char REG_H = 0;    //Memory Register High XX00
unsigned char REG_L = 0;    //Memory Register Low  00XX

//TODO: Add Call, Return, CMP, JMP, etc.
//(CMP could just be a duplicate of SUB, flags are always filled accordingly so JMP, JLT, etc. can use them)

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

unsigned char * getREGTwo(char digitTwo){
    switch(digitTwo){
        case 0x00:
        case 0x08:
            return &ACC;
        case 0x01:
        case 0x09:
            return &REG_B;
        case 0x02:
        case 0x0a:
            return &REG_C;
        case 0x03:
        case 0x0b:
            return &REG_D;
        case 0x04:
        case 0x0c:
            return &REG_E;
        case 0x05:
        case 0x0d:
            return &REG_F;
        case 0x06:
        case 0x0e:
            return &REG_H;
        case 0x07:
        case 0x0f:
        default:
            return &REG_L;
    }
}

void setParity(unsigned char *n)
{
    bool parity = 0;
    while (*n)
    {
        parity = !parity;
        *n = *n & (*n - 1);
    }
    if(parity) {
        REG_F |= 0b00000100;
    }
    else{
        REG_F &= 0b11111011;
    }
}

int ADDSUB(unsigned char OP){
    char digitOne = (OP/0x10)*0x10;
    char digitTwo = OP-(digitOne);
    int isTwoHigh = 0;
    if(digitTwo>0x07){
        isTwoHigh = 1;
    }
    unsigned char *REGOne;
    REGOne = &ACC;
    unsigned char *REGTwo;
    REGTwo = getREGTwo(digitTwo);
    if(isTwoHigh){
        if(*REGTwo == *REGOne){
            *REGOne = 0;
            REG_F = 0b01000000; //set zero flag 1
        }
        else if(*REGTwo > *REGOne){
            *REGOne = *REGTwo-*REGOne;
            REG_F &= 0b00111111; //set sign and zero flags 0
            setParity(REGOne);
        }
        else{
            *REGOne -= *REGTwo;
            REG_F &= 0b00111111; //set sign and zero flags 0
            setParity(REGOne);
        }
    }
    else{
        if(*REGOne + *REGTwo > 255){
            *REGOne = *REGOne + *REGTwo - 255;
            REG_F |= 0b10000001;    //set sign and carry flags 1
            REG_F &= 0b10111111;    //set zero flag 0
            setParity(REGOne);
        }
        else{
            *REGOne = *REGOne+ *REGTwo;
            REG_F |= 0b10000000; //set sign flag 1
            REG_F &= 0b10111110; //set zero and carry flags 0
            setParity(REGOne);
        }
    }
    return 0;
}

int MOV(unsigned char OP){
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
        default:
            return 1;
    }

    REGTwo = getREGTwo(digitTwo);

    *REGOne = *REGTwo;

    return 0;
}

int getInstruction(unsigned char OP){
    if(OP >= 0x30 && OP <= 0x7f){
        return MOV(OP);
    }
    else if (OP >= 0x80 && OP <= 0x8f){
        return ADDSUB(OP);
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
            case LDAL:
                ACC = RAM[ProgramCounter+1];
                ProgramCounter++;
                break;
            case LDA:
                ACC = RAM[MEMADDR];
                ProgramCounter+=2;
                break;
            case STA:
                RAM[MEMADDR] = ACC;
                ProgramCounter+=2;
                break;
            case LDAM:
                ACC = RAM[HLADDR];
                break;
            case STAM:
                RAM[HLADDR] = ACC;
                break;
            case JMP:
                    ProgramCounter = MEMADDR-1;
                break;
            case JNE:
                if(!(REG_F & 0b01000000)){
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case JE:
                if(REG_F & 0b01000000){
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case JGT:
                if((REG_F & 0b01000000) && (REG_F & 0b10000000)){
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case JLT:
                if((REG_F & 0b01000000) && !(REG_F & 0b10000000)){
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
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
