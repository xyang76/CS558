#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MONITOR_RESULT "monitoroutput.txttmp"

int main(int argc, char **argv)
{
    char *msg = "NULL";
    FILE *fp;
    if(argv[1] != NULL){
        msg = argv[1];
    }

    fp=fopen(MONITOR_RESULT,"a+");
    
    //Write a line with message
    fwrite(msg, strlen(msg), 1, fp);
    
    fclose(fp);
}