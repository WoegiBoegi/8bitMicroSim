#include <stdio.h>
#include "CPUSim.h"
#include <ncurses.h>


bool debugModeEnable = false;
bool step = false;
bool halted = false;

WINDOW * winLeft;
WINDOW * winRight;

unsigned char CheckForBIOSInterrupt(){
    return 0x00; //e.g. check if a key is pressed => Keyboard interrupt => ProgramCounter jumps to handling code
}

unsigned char GetInterruptArg(unsigned char INTCODE){
    return 0x00; //e.g. return value of key currently pressed
}

void HandleCPUInterrupt(unsigned char INTCODE, unsigned char ACC){
    //depending on INTCODE, do something with value in ACC, e.g. print char to screen, move cursor, etc.
    if(INTCODE == 0x01){
        printw("%c",ACC);
    }
}

void StartRun(){
    REG_F |= 0b00100000;
    halted = false;
}

void StopRun(){
    REG_F &= 0b11011111;
}

void StopInstr(){
    REG_F &= 0b11011111;
    halted = true;
}

void Error(){
    StopInstr();
    REG_F |= 0b00000010;
}

void printREGs(){
    attroff(COLOR_PAIR(1));
    mvprintw(2,5,"S Z R I O P E C");
    if(REG_F & 0b10000000){
        attron(COLOR_PAIR(1));
        mvprintw(3,5,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,5,"0");
    }
    if(REG_F & 0b01000000){
        attron(COLOR_PAIR(1));
        mvprintw(3,7,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,7,"0");
    }
    if(REG_F & 0b00100000){
        attron(COLOR_PAIR(1));
        mvprintw(3,9,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,9,"0");
    }
    if(REG_F & 0b00010000){
        attron(COLOR_PAIR(1));
        mvprintw(3,11,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,11,"0");
    }
    if(REG_F & 0b00001000){
        attron(COLOR_PAIR(1));
        mvprintw(3,13,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,13,"0");
    }
    if(REG_F & 0b00000100){
        attron(COLOR_PAIR(1));
        mvprintw(3,15,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,15,"0");
    }
    if(REG_F & 0b00000010){
        attron(COLOR_PAIR(1));
        mvprintw(3,17,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,17,"0");
    }
    if(REG_F & 0b00000001){
        attron(COLOR_PAIR(1));
        mvprintw(3,19,"1");
    }
    else{
        attroff(COLOR_PAIR(1));
        mvprintw(3,19,"0");
    }
}

void printRunInfo(bool isRunning, bool hasError){
    if(isRunning){
        attron(COLOR_PAIR(1));
    }
    else{
        attroff(COLOR_PAIR(1));
    }
    mvprintw(2,25,"RUN");

    if(!isRunning){
        attron(COLOR_PAIR(1));
    }
    else{
        attroff(COLOR_PAIR(1));
    }
    mvprintw(2,30,"STOP");

    if(halted){
        attron(COLOR_PAIR(1));
    }
    else{
        attroff(COLOR_PAIR(1));
    }
    mvprintw(2,36,"HALT");

    if(hasError){
        attron(COLOR_PAIR(1));
    }
    else{
        attroff(COLOR_PAIR(1));
    }
    mvprintw(2,42,"ERROR");
}

void printCPUInfo(unsigned char OP, unsigned char Lit, unsigned int HLADDR, unsigned int MEMADDR){
    attroff(COLOR_PAIR(1));
    mvprintw(5,25,"Program Counter: 0x%04x  OP: 0x%02x",ProgramCounter,OP);
    mvprintw(7,25,"next byte Lit:   0x%02x",Lit);
    mvprintw(8,25,"HL Reg Address:  0x%04x",HLADDR);
    mvprintw(9,25,"mem bytes Addr:  0x%04x",MEMADDR);
    mvprintw(11,25,"Stack Pointer:   0x%02x",StackPointer);
    mvprintw(12,25,"Accumulator:     0x%02x",ACC);
    mvprintw(14,25,"Register B:      0x%02x",REG_B);
    mvprintw(15,25,"Register C:      0x%02x",REG_C);
    mvprintw(16,25,"Register D:      0x%02x",REG_D);
    mvprintw(17,25,"Register E:      0x%02x",REG_E);
}

int DoUIStuff(unsigned char OP,unsigned char Lit, unsigned int HLADDR, unsigned int MEMADDR){
    bool isRunning = REG_F & 0b00100000;
    bool hasError = REG_F & 0b00000010;
    if(isRunning && step){
        StopRun();
        step = false;
    }

    clear();


    printREGs();
    printRunInfo(isRunning,hasError);
    printCPUInfo(OP,Lit,HLADDR,MEMADDR);


    refresh();

    timeout(10);
    int ch = getch();
    if(!hasError && ch == KEY_UP){
        StartRun();
    }
    else if(!hasError && ch == KEY_DOWN){
        StopRun();
    }
    else if(!hasError && !isRunning && ch == KEY_RIGHT){
        step = true;
        StartRun();
    }
    else if(!isRunning && ch == 'q'){
        return 0xFF;
    }
    else if(!isRunning && ch == 'm'){
        //do memory dump
    }
    else if(isRunning){
        //keyboard interrupt whatever
    }
    return 0;
}

void StartUI(){
    initscr();			/* Start curses mode 		*/
    raw();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
    noecho();			/* Don't echo() while we do getch */
    printw("8bitMicroSim startup settings:\n");
    printw("Debug Mode [y/N]?");
    start_color();
    init_pair(1,COLOR_RED,COLOR_BLACK);
    refresh();
    int ch = getch();
    if(ch == 'y'){
        debugModeEnable = true;
        printw(" Debug Mode enabled\n");
    }
    else{
        printw((" Debug Mode not enabled\n"));
    }
    refresh();
    //do other settings here
    clear();
    //winLeft = newwin()
}

void StopUI(){
    endwin();
}