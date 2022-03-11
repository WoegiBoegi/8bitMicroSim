#include <stdio.h>

unsigned char CheckForBIOSInterrupt(){
    return 0x00; //e.g. check if a key is pressed => Keyboard interrupt => ProgramCounter jumps to handling code
}

unsigned char GetInterruptArg(unsigned char INTCODE){
    return 0x00; //e.g. return value of key currently pressed
}

void HandleCPUInterrupt(unsigned char INTCODE, unsigned char ACC){
    //depending on INTCODE, do something with value in ACC, e.g. print char to screen, move cursor, etc.
    if(INTCODE == 0x01){
        printf("%c",ACC);
    }
}