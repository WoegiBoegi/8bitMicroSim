#include <stdio.h>
#include <stdbool.h>
#include "BIOSSim.h"
//0x0000 - 0x001F for stack (32 bytes)
//0x0020 - 0x003F for interrupt handling (32 bytes)
#define INTHANDSIZE 0x20
#define MEMSIZE 0x100
#define STACKSIZE 0x20
#define ENTRYPOINT (STACKSIZE + INTHANDSIZE)
#define MEMFILEPATH "/home/woegi/OneDrive/Projects/8bitMicroSim/mem.bin" //"C:\\Users\\woegi\\OneDrive\\Projects\\CPUSim\\mem.bin"

#define NOP  0x00   //no operation, do nothing
#define HLT  0x0F   //halt execution
#define LDA  0x2A   //load value into accumulator from memory address (specified in the next two memory addresses)
#define LDL  0x2B   //load value (in next Memory address) into Accumulator
#define STA  0x2C   //store value from accumulator in memory address (specified in the next two memory addresses)
#define LDM  0x21   //load value (from Memory address specified in REG_H and REG_L) into ACC
#define STM  0x22   //store value from ACC in Memory address (specified in REG_H and REG_L)
#define JMP  0x70   //jumps to address
#define JIE  0x71   //jumps to address if zero flag set
#define JNE  0x72   //jumps to address if zero flag not set
#define JLT  0x7a   //jumps to address if zero flag not set AND sign flag not set
#define JGT  0x7f   //jumps to address if zero flag not set AND sign flag set
#define JIC  0x7d   //jumps to address if carry flag set
#define JHL  0x7e   //jumps to address defined in REG_H and REG_L
#define INA  0xA0   //increment value in Memory address (specified in the next two memory addresses)
#define DEA  0xA1   //decrement value in Memory address (specified in the next two memory addresses)
#define INM  0xAA   //increment value in Memory address (specified in REG_H and REG_L)
#define DEM  0xAB   //decrement value in Memory address (specified in REG_H and REG_L)
#define CAL  0xB0   //jumps to address, saves address to return to later
#define CAE  0xB1   //jumps to address if zero flag set, saves address to return to later
#define CNE  0xB2   //jumps to address if zero flag not set, saves address to return to later
#define CLT  0xBa   //jumps to address if zero flag not set AND sign flag not set, saves address to return to later
#define CGT  0xBb   //jumps to address if zero flag not set AND sign flag set, saves address to return to later
#define CIC  0xBd   //jumps to address if carry flag set, saves address to return to later
#define RET  0xBF   //returns to saved address
#define INT  0x1F   //calls the interrupt in saved in next address
#define RST  0x2F   //soft resets the computer
#define IEN  0x2D   //sets interrupt-enable flag 1
#define IDN  0x2E   //sets interrupt-enable flag 0

/*
 * Cx Stack pointer operations
 * x0 SPR - Stack Pointer Read into ACC
 * x1 SPW - Stack Pointer write from ACC
 * x2 SPL - Push literal value onto Stack
 * x3 SPA - Push value from address onto stack
 * x4 SBA - Pull (bump) value from stack into address
 * x5 SPM - Push value from HLREG address onto stack
 * x6 SBM - Pull (bump) value from stack into HLREG address
 *
 * C0 - C6 reserved for stack pointer operations => e.g. C5 = Push value from HLREG address onto stack
 */

/*
 * ADD AND SUB INSTRUCTION      //CMP is just SUB shush
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
 *
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

/*
 * INC AND DEC INSTRUCTIONS
 * 9x INC || DEC
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
 * 0x90 - 0x9f reserved for INC and DEC operations
 * e.g. 0x94 = INC E = Increment the value in Register E (by one)
 */

/*
 * REGISTER MOV INSTRUCTIONS
 * Cx     || MIL
 * Dx MVO || MVI
 * Ex MOM || MIM
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
 * 0xC8 - 0xEf reserved for REGMOV instructions => e.g. 0xCB = move literal (in next address) into REG_D
 */

/*
 * SPR AND SBR INSTRUCTIONS
 * Fx SPR || SBR
 *
 * A   x0 || x8
 * B   x1 || x9
 * C   x2 || xa
 * D   x3 || xb
 * E   x4 || xc
 * F   x5 || xd
 * H   x6 || xe
 * L   x7 || xf
 *
 * 0xF0 - 0xFF reserved for SPR and SBR => e.g. 0xFC = Pull (bump) value from stack into REG_E
 */

/*
 * CMP INSTRUCTION
 * 1X CMP A
 * X0     A
 * X1     B
 * X2     C
 * X3     D
 * X4     E
 * X5     F
 * X6     H
 * X7     L
 *
 * 0x10 - 0x17 reserved for CMP => e.g. 0x13 = Compare REG_A with REG_D
 */

unsigned char StackPointer = STACKSIZE-1; //0x0000 - STACKSIZE reserved for stack
unsigned int ProgramCounter = ENTRYPOINT; //starts at ENTRYPOINT
unsigned char ACC = 0;
unsigned char REG_B = 0;
unsigned char REG_C = 0;
unsigned char REG_D = 0;
unsigned char REG_E = 0;

/*Flag Register
*  7 6 5 4 3 2 1 0
*  S Z 0 I 0 P 0 C
*  S: Sign Flag, 1 if result of previous operation is positive
*  Z: Zero Flag, 1 if result of previous operation is zero
*  P: Parity indicates amount of 1s in operation in binary, 1 if result has even amount of 1s
*  C: Carry Flag, if carry was perfored in most significant bit i.e. 254+6 = 260 > 255 => C = 1
*  I: Interrupt enable/disable 1 = interrupt enabled
*  0: unused, always zero
*/
unsigned char REG_F = 0;

unsigned char REG_H = 0;    //Memory Register High XX00
unsigned char REG_L = 0;    //Memory Register Low  00XX

//TODO: Debug mode, Assembler, Programmer's Manual

unsigned char MEM[MEMSIZE];

void printMEM(){
    printf("--------------------------------------------------------\n");
    printf("      00 01 02 03  04 05 06 07  08 09 0a 0b  0c 0d 0e 0f\n");

    for(int ADDR = 0x00; ADDR <= (MEMSIZE-0x10); ADDR += 0x10){
        printf("%04x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x\n", ADDR, MEM[ADDR + 0x00], MEM[ADDR + 0x01], MEM[ADDR + 0x02], MEM[ADDR + 0x03], MEM[ADDR + 0x04], MEM[ADDR + 0x05], MEM[ADDR + 0x06], MEM[ADDR + 0x07], MEM[ADDR + 0x08], MEM[ADDR + 0x09], MEM[ADDR + 0x0a], MEM[ADDR + 0x0b], MEM[ADDR + 0x0c], MEM[ADDR + 0x0d], MEM[ADDR + 0x0e], MEM[ADDR + 0x0f]);
    }
    printf("--------------------------------------------------------\n");
}

void printDebugInfo(){
    printf("\n========================================================\n");
    printf("EXECUTION HALTED: DUMPING ALL INFO:\n");
    //print all registers, flags, etc. as well
    printf("program counter at position %04x\nstack pointer at position %04x\n",ProgramCounter,StackPointer);
    printf("value in Accumulator: %02x\nvalue in Register B:  %02x\n"
           "value in Register C:  %02x\nvalue in Register D:  %02x\nvalue in Register E:  %02x\n"
           "value in Register F:  %02x\nvalue in Register H:  %02x\nvalue in Register L:  %02x\n",
           ACC,REG_B,REG_C,REG_D,REG_E,REG_F,REG_H,REG_L);
    printf("dumping Memory:\n");
    printMEM();
    printf("========================================================\n");
}

void PushOnStack(unsigned char value){
    MEM[StackPointer] = value;
    if(StackPointer == 0){
        StackPointer = STACKSIZE-1;
        printf("STACKUNDERFLOW OCCURED\n");
    }
    else{
        StackPointer--;
    }
}

unsigned char PullFromStack(){
    unsigned char value = MEM[StackPointer];
    if(StackPointer == STACKSIZE-1){
        StackPointer = 0;
        printf("STACKOVERFLOW OCCURED\n");
    }
    else{
        StackPointer++;
    }
    return value;
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

void setParity(unsigned char *val)
{
    unsigned char n = *val;
    bool parity = 0;
    while (n)
    {
        parity = !parity;
        n = n & (n - 1);
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
            REG_F |= 0b01000000; //set zero flag 1
            REG_F &= 0b01111110; //set sign and carry flags 0
        }
        else if(*REGTwo > *REGOne){
            *REGOne = *REGTwo-*REGOne;
            REG_F &= 0b00111110; //set sign, zero and carry flags 0
            setParity(REGOne);
        }
        else{
            *REGOne -= *REGTwo;
            REG_F |= 0b10000000; //set sign flag 1 since result is positive
            REG_F &= 0b10111110; //set zero and carry flags 0
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

int CMP(unsigned char OP){
    char digitOne = (OP/0x10)*0x10;
    char digitTwo = OP-(digitOne);

    unsigned char *REGOne;
    REGOne = &ACC;
    unsigned char *REGTwo;
    REGTwo = getREGTwo(digitTwo);

    if(*REGTwo > *REGOne){
        REG_F &= 0b00111110; //set sign, zero and carry flags 0
    }
    else if (*REGTwo == *REGOne){
        REG_F |= 0b01000000; //set zero flag 1
        REG_F &= 0b01111110; //set sign and carry flags 0
    }
    else{ // REGTwo can only be less than REGOne
        REG_F |= 0b10000000; //set sign flag 1 since result is positive
        REG_F &= 0b10111110; //set zero and carry flags 0
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
    }

    REGTwo = getREGTwo(digitTwo);

    *REGOne = *REGTwo;

    return 0;
}

int INCDEC(unsigned char OP){
    char digitOne = (OP/0x10)*0x10;
    char digitTwo = OP-(digitOne);
    int isTwoHigh = 0;
    if(digitTwo>0x07){
        isTwoHigh = 1;
    }

    unsigned char *REG = getREGTwo(digitTwo);

    if(isTwoHigh){ //digit two is between 0x8 and 0xf => decrement register
        if(*REG == 0x00){ // if REG is 0x00, increment it and unset sign flag instead
            *REG += 1;
            REG_F &= 0b01111111;
        }
        else{ //decrement REG normally
            *REG -= 1;
        }
    }
    else{ //increment register
        if(*REG == 0xFF){ // if REG is 0xFF, set it to 0x00 and set carry flag instead
            *REG = 0x00;
            REG_F |= 0b00000001;
        }
        else{ //increment REG normally
            *REG += 1;
        }
    }
    if(*REG == 0){
        REG_F |= 0b01000000;
    }
    else{
        REG_F &= 0b10111111;
    }
    setParity(REG);
    return 0;
}

int StackOP(unsigned char OP, unsigned int MEMADDR, unsigned int HLADDR, unsigned char nextLIT){
    char digitOne = (OP/0x10)*0x10;
    char digitTwo = OP-(digitOne);

    switch (digitTwo) {
        case 0x0:
            ACC = StackPointer;
            break;
        case 0x1:
            StackPointer = ACC;
            break;
        case 0x2:
            PushOnStack(nextLIT);
            ProgramCounter++;
            break;
        case 0x3:
            PushOnStack(MEM[MEMADDR]);
            ProgramCounter+=2;
            break;
        case 0x4:
            MEM[MEMADDR] = PullFromStack();
            ProgramCounter+=2;
            break;
        case 0x5:
            PushOnStack(MEM[HLADDR]);
            break;
        case 0x6:
            MEM[HLADDR] = PullFromStack();
            break;
    }
    return 0;
}

int REGMOV(unsigned char OP, unsigned int MEMADDR, unsigned int HLADDR, unsigned char nextLIT){
    unsigned char digitOne = (OP/0x10)*0x10;
    char digitTwo = OP-(digitOne);
    int isTwoHigh = 0;
    if(digitTwo>0x07){
        isTwoHigh = 1;
    }

    unsigned char *REG = getREGTwo(digitTwo);
    if(digitOne == 0xC0){
        *REG = nextLIT;
        ProgramCounter++;
    }
    else if(digitOne == 0xD0){
        if(isTwoHigh){
            *REG = MEM[MEMADDR];
        }
        else{
            MEM[MEMADDR] = *REG;
        }
        ProgramCounter+=2;
    }
    else if(digitOne == 0xE0){
        if(isTwoHigh){
            *REG = MEM[HLADDR];
        }
        else{
            MEM[HLADDR] = *REG;
        }
        ProgramCounter+=2;
    }
    return 0;
}

int SOPREG(unsigned char OP){
    char digitOne = (OP/0x10)*0x10;
    char digitTwo = OP-(digitOne);
    int isTwoHigh = 0;
    if(digitTwo>0x07){
        isTwoHigh = 1;
    }

    unsigned char *REG = getREGTwo(digitTwo);
    if(isTwoHigh){
        *REG = PullFromStack();
    }
    else{
        PushOnStack(*REG);
    }
    return 0;
}

int getInstruction(unsigned char OP, unsigned int MEMADDR, unsigned int HLADDR, unsigned char nextLIT){
    if(OP >= 0x10 && OP <= 0x17){
        return CMP(OP);
    }
    else if(OP >= 0x30 && OP <= 0x6f){
        return MOV(OP);
    }
    else if (OP >= 0x80 && OP <= 0x8f){
        return ADDSUB(OP);
    }
    else if (OP >= 0x90 && OP <= 0x9f){
        return INCDEC(OP);
    }
    else if (OP >= 0xC0 && OP <= 0xC6){
        return StackOP(OP, MEMADDR, HLADDR, nextLIT);
    }
    else if (OP >= 0xC8 && OP <= 0xEF){
        return REGMOV(OP, MEMADDR, HLADDR, nextLIT);
    }
    else if (OP >= 0xF0 && OP <= 0xFF){
        return SOPREG(OP);
    }
    else {
        return 1;
    }
}

void Push16bitValInStack(unsigned int value){
    unsigned char HighByte = value/0x100;
    unsigned char LowByte = value-HighByte;
    PushOnStack(LowByte);
    PushOnStack(HighByte);
}

void HandleBIOSInterrupt(unsigned char INTCODE){
    Push16bitValInStack(ProgramCounter);
    PushOnStack(ACC);
    ACC = GetInterruptArg(INTCODE);
    ProgramCounter = STACKSIZE + ((INTCODE-1)*0xF);
    //set ProgramCounter to Address that corresponds to interrupt handling code...
}

unsigned int Pull16bitValFromStack(){
    unsigned char LowByte = PullFromStack();
    unsigned char HighByte = PullFromStack();
    unsigned int value = HighByte*0x100+LowByte;
    return value;
}

int MainLoop(){
    while(1){
        unsigned char INTCODE = CheckForBIOSInterrupt();
        if(INTCODE != 0x00 && (REG_F & 0b00010000)){
            HandleBIOSInterrupt(INTCODE);
        }
        unsigned char OP = MEM[ProgramCounter];
        unsigned int HLADDR = REG_H*0x100+REG_L;
        unsigned int MEMADDR = MEM[ProgramCounter + 1] * 0x100 + MEM[ProgramCounter + 2];
        switch(OP){
            case NOP:
                break;
            case HLT:
                printDebugInfo();
                return 0;
            case RST:
                StackPointer = STACKSIZE-1; 
                ProgramCounter = ENTRYPOINT-1;
                ACC = 0;
                REG_B = 0;
                REG_C = 0;
                REG_D = 0;
                REG_E = 0;
                break;
            case IEN:
                REG_F |= 0b00010000;
                break;
            case IDN:
                REG_F &= 0b11101111;
                break;
            case INT:
                HandleCPUInterrupt(MEM[ProgramCounter + 1], ACC);
                ProgramCounter++;
                break;
            case LDL:
                ACC = MEM[ProgramCounter + 1];
                ProgramCounter++;
                break;
            case LDA:
                ACC = MEM[MEMADDR];
                ProgramCounter+=2;
                break;
            case STA:
                MEM[MEMADDR] = ACC;
                ProgramCounter+=2;
                break;
            case LDM:
                ACC = MEM[HLADDR];
                break;
            case STM:
                MEM[HLADDR] = ACC;
                break;
            case JHL:
                ProgramCounter = HLADDR-1;
                break;
            case JMP:
            case CAL:
                if(OP == CAL){
                    Push16bitValInStack(ProgramCounter+2);
                }
                ProgramCounter = MEMADDR-1;
                break;
            case JNE:
            case CNE:
                if(!(REG_F & 0b01000000)){
                    if(OP == CNE){
                        Push16bitValInStack(ProgramCounter+2);
                    }
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case JIE:
            case CAE:
                if(REG_F & 0b01000000){
                    if(OP == CAE){
                        Push16bitValInStack(ProgramCounter+2);
                    }
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case JGT:
            case CGT:
                if(!(REG_F & 0b01000000) && (REG_F & 0b10000000)){
                    if(OP == CGT){
                        Push16bitValInStack(ProgramCounter+2);
                    }
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case JLT:
            case CLT:
                if(!(REG_F & 0b01000000) && !(REG_F & 0b10000000)){
                    if(OP == CLT){
                        Push16bitValInStack(ProgramCounter+2);
                    }
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case JIC:
            case CIC:
                if(REG_F &0b00000001){
                    if(OP == CIC){
                        Push16bitValInStack(ProgramCounter+2);
                    }
                    ProgramCounter = MEMADDR-1;
                }
                else{
                    ProgramCounter+=2;
                }
                break;
            case RET:
                ProgramCounter = Pull16bitValFromStack();
                break;
            case INA:
                if(MEM[MEMADDR] == 0xFF){
                    MEM[MEMADDR] = 0x00;
                    REG_F |= 0b00000001;
                }
                else{
                    MEM[MEMADDR] += 1;
                }
                setParity(&MEM[MEMADDR]);
                ProgramCounter+=2;
                break;
            case DEA:
                if(MEM[MEMADDR] == 0x00){
                    MEM[MEMADDR] += 1;
                    REG_F &= 0b01111111;
                }
                else{
                    MEM[MEMADDR] -=1;
                    if(MEM[MEMADDR] == 0x00){
                        REG_F |= 0b01000000;
                    }
                    else{
                        REG_F &= 0b10111111;
                    }
                }
                setParity(&MEM[MEMADDR]);
                ProgramCounter+=2;
                break;
            case INM:
                if(MEM[HLADDR] == 0xFF){
                    MEM[HLADDR] = 0x00;
                    REG_F &= 0b00000001;
                }
                else{
                    MEM[HLADDR] += 1;
                }
                setParity(&MEM[HLADDR]);
                break;
            case DEM:
                if(MEM[HLADDR] == 0x00){
                    MEM[HLADDR] += 1;
                    REG_F &= 0b01111111;
                }
                else{
                    MEM[HLADDR] -=1;
                    if(MEM[HLADDR] == 0x00){
                        REG_F |= 0b01000000;
                    }
                    else{
                        REG_F &= 0b10111111;
                    }
                }
                setParity(&MEM[HLADDR]);
                break;
            default:

                if(getInstruction(OP,MEMADDR,HLADDR,MEM[ProgramCounter+1])){
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
    fread(MEM, sizeof(MEM), 1, src);

    int ret = MainLoop();

    if(ret != 0){
        printf("PROGRAM HAS ENCOUNTERED AN ERROR, SEE INFODUMP FOR DETAILS\n");
    }
    else{
        printf("Program has executed without errors, dumped info for debug reasons :)\n");
    }
    return 0;
}
