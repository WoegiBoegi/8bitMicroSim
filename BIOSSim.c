unsigned char CheckForBIOSInterrupt(){
    return 0x00; //e.g. check if a key is pressed => Keyboard interrupt => ProgramCounter jumps to handling code
}

void HandleCPUInterrupt(unsigned char INTCODE, unsigned char ACC){
    //depending on INTCODE, do something with value in ACC, e.g. print char to screen, move cursor, etc.
}