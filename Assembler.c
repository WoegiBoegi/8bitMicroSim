#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MEMSIZE 0x10000

unsigned char MEM[MEMSIZE];
unsigned int MEMLOC = 0;


unsigned int getNum(char *line){
    int base = 2;
    if(line[0] == 'x'){     //x is the prefix for hexadecimal
        base = 16;
    }
    else if(line[0] == 'd'){ //d = decimal
        base = 10;
    } 
    return (int)strtol(line+1,NULL,base); 
}

void parseLine(char *line){
    if(line[0] == ';'){ //ignore line if it's a comment 
        return;
    }
    else if(line[0] == '$'){    //$means "set location to"
        MEMLOC = getNum(line+1);
        printf("set memloc to %04x\n",MEMLOC);
        return;
    }
    else{   //time to get the Instruction from the mnemonic...
        if(strcmp(line, "HLT") == 0){
            MEM[MEMLOC] = 0x01;
            printf("inserted value 0x01 into %04x\n",MEMLOC);
        }
    }

    MEMLOC++;
    //parse line to do shit ig
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
        parseLine(line);
    }
    fclose(fp);

    fp = fopen(outName,"w");
    fwrite(MEM,1,sizeof(MEM),fp);
    fclose(fp);
    //write MEM to file

    
    exit(EXIT_SUCCESS);
}