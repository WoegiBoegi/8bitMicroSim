#include <stdio.h>
#include "CPUSim.h"
#include <ncurses.h>

bool KeyPressed = false;

bool displayCPUdata = false;
bool displayMEMdata = false;
bool displayTERMdata = false;
bool step = false;
bool halted = false;

int timeoutVAL = 20;

unsigned int virtualPC = 0;

#define TERMLINES 24
#define TERMCOLS 80

unsigned char KEYVAL = 0;


unsigned char TerminalBuffer[TERMLINES][TERMCOLS]; //24 lines, 80 collumns
int CursorLine = 0;
int CursorCol = 0;

void Scroll(){
    CursorLine--;
    for(int line = 0; line < TERMLINES-1; line++){
        for(int col = 0; col < TERMCOLS; col++){
            TerminalBuffer[line][col] = TerminalBuffer[line+1][col];
        }
    }
    for(int col = 0; col < TERMCOLS; col++){
        TerminalBuffer[TERMLINES-1][col] = 0;
    }
}

void AdvCursor(){
    CursorCol++;
    if(CursorCol >= TERMCOLS){
        CursorCol = 0;
        CursorLine++;
        if(CursorLine >= TERMLINES){
            Scroll();
        }
    }
}

void NewLine(){
    CursorCol = 0;
    CursorLine++;
    if(CursorLine >= TERMLINES){
        Scroll();
    }
}

void ClearTermBuffer(){
    for(int line = 0; line < TERMLINES; line++){
        for(int col = 0; col < TERMCOLS; col++){
            TerminalBuffer[line][col] = 0;
        }
    }
    CursorLine = 0;
    CursorCol = 0;
}

void rectangle(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2-x1);
    mvhline(y2, x1, 0, x2-x1);
    mvvline(y1, x1, 0, y2-y1);
    mvvline(y1, x2, 0, y2-y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

void StartRun(){
    REG_F |= 0b00100000;
    halted = false;
    KeyPressed = true;
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

unsigned char CheckForBIOSInterrupt(){
    timeout(timeoutVAL);
    int ch = getch();
    if(ch == KEY_DOWN){
        StopRun();
        return 0xFF;
    }
    else if(ch != -1 && !KeyPressed && displayTERMdata){
        KeyPressed = true;
        KEYVAL = ch;
        return 0x01;
    }
    else if(KeyPressed && ch == -1 && displayTERMdata){
        KeyPressed = false;
    }
    return 0x00; //e.g. check if a key is pressed => Keyboard interrupt => ProgramCounter jumps to handling code
}

unsigned char GetInterruptArg(unsigned char INTCODE){
    if(INTCODE == 0x01 && displayTERMdata){
        return KEYVAL;
    }
    return 0x00; //e.g. return value of key currently pressed
}

void HandleCPUInterrupt(unsigned char INTCODE, unsigned char ACC){
    //depending on INTCODE, do something with value in ACC, e.g. print char to screen, move cursor, etc.
    if(INTCODE == 0x01){
        TerminalBuffer[CursorLine][CursorCol] = ACC;
        if(ACC == '\n'){
            NewLine();
        }
        else{
            AdvCursor();
        }
    }
}

void drawLine(int col){
    for(int y = 0; y <= LINES;y++){
        mvprintw(y,col,"|");
    }
}

void printMEM(unsigned int PC){
    int maxAddrLine = 0xFFF;
    int lines = LINES-8;
    int addrLine = (PC/0x10);
    int lineBuffer = (lines-1)/2;
    int lineBufferPre = lineBuffer;
    int lineBufferPost = lineBuffer;
    if(lineBuffer > addrLine){
        lineBufferPre = addrLine-1;
        lineBufferPost = lines-1-lineBufferPre;
    }
    else if(lineBuffer > maxAddrLine-addrLine){
        lineBufferPost = maxAddrLine-addrLine-1;
        lineBufferPre = lines-1-lineBufferPost;
    }

    int firstLine = addrLine-lineBufferPre-1;
    int lastLine = addrLine+lineBufferPost;
    int x = 80;
    int y = 3;
    for(int relLine = 0; firstLine+relLine <= lastLine; relLine++){
        unsigned int lineAddr = (firstLine+relLine)*0x10;
        attroff(COLOR_PAIR(1));
        mvprintw(y+relLine,x-8,"0x%04x ",lineAddr);
        for(int relAddr = 0; relAddr <= 0xF; relAddr++){
            unsigned int addr = lineAddr+relAddr;
            attroff(COLOR_PAIR(1));
            if(addr == ProgramCounter){
                attron(COLOR_PAIR(1));
            }
            mvprintw(y+relLine,x+relAddr*3,"%02x",MEM[addr]);
        }
    }

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

void printTermBuffer(){
    rectangle(2,139,27,220);
    mvprintw(2,142,"TERMINAL");
    mvprintw(27,210,"TERMINAL");
    int yOffset = 3;
    int xOffset = 140;
    for(int line = 0; line < TERMLINES; line++){
        for(int col = 0; col < TERMCOLS; col++){
            if(line == CursorLine && col == CursorCol){
                mvprintw(line+yOffset,xOffset+col,"_");
            }
            else{
                mvprintw(line+yOffset,xOffset+col,"%c",TerminalBuffer[line][col]);
            }
        }
    }
}

int DoUIStuff(unsigned char OP,unsigned char Lit, unsigned int HLADDR, unsigned int MEMADDR){
    bool isRunning = REG_F & 0b00100000;
    bool hasError = REG_F & 0b00000010;
    if(isRunning && step){
        StopRun();
        step = false;
    }

    clear();
    if(displayCPUdata){
        printREGs();
        printRunInfo(isRunning,hasError);
        printCPUInfo(OP,Lit,HLADDR,MEMADDR);
    }
    mvprintw(LINES-7,10,"delay in ms: %d",timeoutVAL);
    mvprintw(LINES-5,10,"[q]-quit  [f]-faster  [s]-slower");
    mvprintw(LINES-3,4,"UPARROW-run  DOWNARROW-stop  RIGHTARROW-step");
    drawLine(65);
    if(displayMEMdata){
        mvprintw(LINES-3,85,"[o]-scroll up [l]-scroll down");
        if(isRunning){
            virtualPC = ProgramCounter;
            printMEM(ProgramCounter);
        }
        else{
            printMEM(virtualPC);
        }
    }
    drawLine(133);
    if(displayTERMdata){
        printTermBuffer();
    }
    refresh();
    if(!isRunning){
        timeout(50);
        int ch = getch();
        if(!hasError && ch == KEY_UP){
            StartRun();
        }
        else if(!hasError && ch == KEY_RIGHT){
            step = true;
            StartRun();
        }
        else if(ch == 'q'){
            return 0xFF;
        }
        else if(ch == 'r'){
            ClearTermBuffer();
            HardReset();
        }
        else if(ch == 'f'){
            timeoutVAL--;
        }
        else if (ch == 's'){
            timeoutVAL++;
        }
        else if (ch == 'o'){
            if(virtualPC > 0x50){
                virtualPC -= 0x50;
            }
        }
        else if(ch == 'l'){
            if(virtualPC < 0xFFFF - 0x50){
                virtualPC += 0x50;
            }
        }
    }

    return 0;
}

void StartupSettings(){
    printw("Display CPU info? [Y/n] ");
    refresh();
    int ch = getch();
    if(ch == 'n'){
        printw(" N\n");
    }
    else{
        displayCPUdata = true;
        printw(" Y\n");
    }
    printw("Display Memory info? [Y/n] ");
    refresh();
    ch = getch();
    if(ch == 'n'){
        printw(" N\n");
    }
    else{
        displayMEMdata = true;
        printw(" Y\n");
    }
    printw("Enable Terminal Emulation? [Y/n] ");
    refresh();
    ch = getch();
    if(ch == 'n'){
        printw(" N\n");
    }
    else{
        displayTERMdata = true;
        printw(" Y\n");
    }
    refresh();
}

void StartUI(){
    initscr();			/* Start curses mode 		*/
    raw();				/* Line buffering disabled	*/
    curs_set(0);
    keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
    noecho();			/* Don't echo() while we do getch */
    printw("8bitMicroSim startup settings:\n");
    start_color();
    init_pair(1,COLOR_RED,COLOR_BLACK);
    StartupSettings();
    clear();
    virtualPC = ProgramCounter;
}

void StopUI(){
    endwin();
}