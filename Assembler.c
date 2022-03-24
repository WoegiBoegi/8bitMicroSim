#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MEMSIZE 0x10000

unsigned char MEM[MEMSIZE];
unsigned int MEMLOC = 0;

unsigned int labelcount = 0;
char *labels[100];
unsigned int pointers[100];

bool firstPass = true;

unsigned int getNum(char *line){
    if(line[0] == ':'){
        if(firstPass == true){
            return 0xFFFF;
        }
        for(int n = 0; n < 100; n++){
            if(labels[n]){
                if(strcmp(labels[n], line+1) == 0){
                    return pointers[n];
                }
            }
        }
        printf("invalid label: %s\n",line);
        return 0; 
    }
    int base = 10;
    if(line[0] == 'x'){     //x is the prefix for hexadecimal
        base = 16;
    }
    else if(line[0] == 'b'){ //b = binary, default is decimal
        base = 2;
    } 
    return (int)strtol(line+1,NULL,base); 
}

void setByte(char *line){
    unsigned int num = getNum(line);
    MEM[++MEMLOC] = num;
}

void setAddr(char *line){
    unsigned int addr = getNum(line);   
    unsigned char highByte = (addr/0x100);
    unsigned char lowByte = addr-(highByte*0x100);
    MEM[++MEMLOC] = highByte;
    MEM[++MEMLOC] = lowByte;
}

int getREGVal(char r){
    if(r == 'H')
        r = 'G';
    else if (r == 'L') //reassign chars so that the math works out
        r = 'H';
    return r-'A';
}

void parseLine(char *line){
    if(line[0] == ';'){ //ignore line if it's a comment 
        return;
    }
    else if (strncmp(line,"l:",2) == 0){
        MEM[MEMLOC] = getNum(line+2);
    }
    else if(strncmp(line,"w:",2) == 0){
        for(int n = 2; n < strlen(line);n++){
            MEM[MEMLOC++] = line[n];
        }
        return;
    }
    else if(line[0] == ':'){
        if(firstPass == false){
            return;
        }
        char *newLabel = malloc(strlen(line));
        for(int n = 1; n < strlen(line);n++){
            newLabel[n-1] = line[n];
        }
        newLabel[strlen(line)-1] = '\0';
        labels[labelcount] = newLabel;
        pointers[labelcount] = MEMLOC;
        labelcount++;
        return;
    }
    else if(line[0] == '$'){    //$ means "set location to"
        MEMLOC = getNum(line+1);
        return;
    }
    else{   //time to get the Instruction from the mnemonic...
        if(strncmp(line, "HLT",3) == 0){
            MEM[MEMLOC] = 0x01;
        }
        else if(strncmp(line, "NOP",3) == 0) {
            MEM[MEMLOC] = 0x00;
        }
        else if(strncmp(line, "IEN",3) == 0) {
            MEM[MEMLOC] = 0x02;
        }
        else if(strncmp(line, "IDN",3) == 0) {
            MEM[MEMLOC] = 0x03;
        }
        else if(strncmp(line, "RST",3) == 0) {
            MEM[MEMLOC] = 0x04;
        }
        else if(strncmp(line, "LDM", 3) == 0){
            MEM[MEMLOC] = 0x20;
        }
        else if(strncmp(line, "STM", 3) == 0){
            MEM[MEMLOC] = 0x21;
        }
        else if(strncmp(line, "LDL", 3) == 0){
            MEM[MEMLOC] = 0x23;
            setByte(line+3);
        }
        else if(strncmp(line, "LDA", 3) == 0){
            MEM[MEMLOC] = 0x22;
            setAddr(line+3);
        }
        else if(strncmp(line, "STA", 3) == 0){
            MEM[MEMLOC] = 0x24;
            setAddr(line+3);
        }
        else if(strncmp(line, "INT",3) == 0) {
            MEM[MEMLOC] = 0x04+getNum(line+3);
        }
        else if(strncmp(line,"MOV", 3) == 0){
            MEM[MEMLOC] = 0x30+(0x08*getREGVal(line[3])+getREGVal(line[4]));
        }
        else if(strncmp(line, "CMP", 3) == 0){
            MEM[MEMLOC] = 0x10+getREGVal(line[3]);
        }
        else if(strncmp(line, "NND", 3) == 0){
            MEM[MEMLOC] = 0x18+getREGVal(line[3]);
        }
        else if(strncmp(line, "NOT", 3) == 0){
            MEM[MEMLOC] = 0x28+getREGVal(line[3]);
        }
        else if(strncmp(line, "JHL",3) == 0){
            MEM[MEMLOC] = 0x75;
        }
        else if(strncmp(line, "J", 1) == 0){
            int opcode = 0x70;          
            if(strncmp(line+1, "IE", 2) == 0)
                opcode+=1;
            else if(strncmp(line+1, "NE", 2) == 0)
                opcode+=2;
            else if(strncmp(line+1, "LT", 2) == 0)
                opcode+=3;
            else if(strncmp(line+1, "IC", 2) == 0)
                opcode+=4;
            else if(strncmp(line+1, "GT", 2) == 0)
                opcode+=6;
            
            MEM[MEMLOC] = opcode;
            setAddr(line+3);
        }
        else if(strncmp(line, "AND", 3) == 0){
            MEM[MEMLOC] = 0x78+getREGVal(line[3]);
        }
        else if(strncmp(line, "ORA", 3) == 0){
            MEM[MEMLOC] = 0xA8+getREGVal(line[3]);
        }
        else if(strncmp(line, "XOR", 3) == 0){
            MEM[MEMLOC] = 0xB8+getREGVal(line[3]);
        }
        else if(strncmp(line, "ADD", 3) == 0){
            MEM[MEMLOC] = 0x80+getREGVal(line[3]);
        }
        else if(strncmp(line, "SUB", 3) == 0){
            MEM[MEMLOC] = 0x88+getREGVal(line[3]);
        }
        else if(strncmp(line, "INC", 3) == 0){
            MEM[MEMLOC] = 0x90+getREGVal(line[3]);
        }
        else if(strncmp(line, "DEC", 3) == 0){
            MEM[MEMLOC] = 0x98+getREGVal(line[3]);
        }
        else if(strncmp(line, "INA", 3) == 0){
            MEM[MEMLOC] = 0xA0;
            setAddr(line+3);
        }
        else if(strncmp(line, "DEA", 3) == 0){
            MEM[MEMLOC] = 0xA1;
            setAddr(line+3);
        }
        else if(strncmp(line, "INM", 3) == 0){
            MEM[MEMLOC] = 0xA2;
        }
        else if(strncmp(line, "DEM", 3) == 0){
            MEM[MEMLOC] = 0xA3;
        }
        else if(strncmp(line, "RET", 3) == 0){
            MEM[MEMLOC] = 0xB7;
        }
        else if(strncmp(line, "CHL", 3) == 0){
            MEM[MEMLOC] = 0xB6;
        }
        else if(strncmp(line, "C", 1) == 0){
            int opcode = 0xB0;          
            if(strncmp(line+1, "IE", 2) == 0)
                opcode+=1;
            else if(strncmp(line+1, "NE", 2) == 0)
                opcode+=2;
            else if(strncmp(line+1, "LT", 2) == 0)
                opcode+=3;
            else if(strncmp(line+1, "IC", 2) == 0)
                opcode+=5;
            else if(strncmp(line+1, "GT", 2) == 0)
                opcode+=4;
            
            MEM[MEMLOC] = opcode;
            setAddr(line+3);
        }
        else if(strncmp(line, "SPS", 3) == 0){
            MEM[MEMLOC] = 0xC0;
        }
        else if(strncmp(line, "SPW", 3) == 0){
            MEM[MEMLOC] = 0xC1;
        }
        else if(strncmp(line, "SPL", 3) == 0){
            MEM[MEMLOC] = 0xC2;
            setByte(line+3);
        }
        else if(strncmp(line, "SPA", 3) == 0){
            MEM[MEMLOC] = 0xC3;
            setAddr(line+3);
        }
        else if(strncmp(line, "SBA", 3) == 0){
            MEM[MEMLOC] = 0xC4;
            setAddr(line+3);
        }
        else if(strncmp(line, "SPM", 3) == 0){
            MEM[MEMLOC] = 0xC5;
        }
        else if(strncmp(line, "MIL", 3) == 0){
            MEM[MEMLOC] = 0xC8+getREGVal(line[3]);
            setByte(line+4);
        }
        else if(strncmp(line, "MOM", 3) == 0){
            MEM[MEMLOC] = 0xE0+getREGVal(line[3]);
        }
        else if(strncmp(line, "MIM", 3) == 0){
            MEM[MEMLOC] = 0xE8+getREGVal(line[3]);
        }
        else if(strncmp(line, "SPR", 3) == 0){
            MEM[MEMLOC] = 0xF0+getREGVal(line[3]);
        }
        else if(strncmp(line, "SBR", 3) == 0){
            MEM[MEMLOC] = 0xF8+getREGVal(line[3]);
        }
        else if(strncmp(line, "LHL", 3) == 0){
            MEM[MEMLOC] = 0x25;
            setAddr(line + 3);
        }
        else if(strncmp(line, "SHL", 3) == 0){
            MEM[MEMLOC] = 0x26;
            setAddr(line + 3);
        }
    }

    MEMLOC++;
}

void filterLine(char *s) {
    char *d = s;
    int filter = 1;
    do {
        while (*d == ' ' || *d == ',' || *d == '"') {
            if(*d == '"'){
                d++;
                filter = !filter;
            }
            else{
                if(filter == 1)
                    d++;
                if(filter == 0)
                    break;
            }
        }

    } while ((*s++ = *d++));
}

void assembleFromFile(char * inName)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(inName, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if(line[strlen(line)-1] == '\n')
            line[strlen(line)-1] = '\0';
        filterLine(line);
        parseLine(line);
    }
    fclose(fp);
}


int main(int argc, char *argv[]){

    for(int n = 0; n < MEMSIZE; n++){
        MEM[n] = 0x00;
    }
    char *outName;
    char *inName;
    if(argc <= 1){
        inName = "./test.asm";
    }
    else{
        inName = argv[1];
    }

    if(argc <= 2){
        outName = "out.bin";
    }
    else{
        outName = argv[2];
    }
    
    assembleFromFile(inName);
    firstPass = false;
    assembleFromFile(inName);

    FILE * fp = fopen(outName,"w");
    fwrite(MEM,1,sizeof(MEM),fp);
    fclose(fp);
    //write MEM to file

    for(int n = 0; n < 100; n++){
        if(labels[n])
            free(labels[n]);
    }
    
    exit(EXIT_SUCCESS);
}