#include <stdio.h>
#include <stdlib.h>
#define MEMSIZE 0x10000

unsigned char MEM[MEMSIZE];
void parseLine(char *line){
    //parse line to do shit ig
}



int main(int argc, char *argv[]){

    for(int n = 0; n < MEMSIZE; n++){
        MEM[n] = 0x00;
    }

    if(argc <= 1){
        printf("please specify a file to be assembled\n");
        return 1;
    }
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(argv[1], "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        parseLine(line);
    }

    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}