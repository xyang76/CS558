#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MONITOR_RESULT "/usr/CS558/Demo/monitoroutput.txttmp"

int main(int argc, char **argv)
{
    char *msg = "NULL\n";
    FILE *fp;
    
    fp=fopen(MONITOR_RESULT,"a+");
    
    if(argc > 1){
        fwrite("yes3:", 5, 1, fp);
        fwrite(argv[1], strlen(argv[1]), 1, fp);
    } else if (argc == 1){
        fwrite("yes1:", 5, 1, fp);
        fwrite(argv[0], strlen(argv[0]), 1, fp);
    } else {
        fwrite("yes2:", 5, 1, fp);
        fwrite(msg, strlen(msg), 1, fp);
    }
    
    fclose(fp);
}