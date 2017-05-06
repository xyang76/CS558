#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MONITOR_RESULT "monitoroutput.txttmp"

int main(int argc, char **argv)
{
    char *type, *msg;
    FILE *fp;
    if(argc == 3){
        type = argv[1];
        msg = argv[2];
        
        fp=fopen(MONITOR_RESULT,"a+");
        
        //Write a line with message
        fwrite(type, strlen(type), 1, fp);
        fputc(' ', fp);
        fwrite(msg, strlen(msg), 1, fp);
        fputc('\n', fp);
        
        fclose(fp);
    }
}