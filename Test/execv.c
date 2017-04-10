#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h" 

void printmsg(char*msg);

int main(int argc, char **argv)
{
    printmsg("Hello");
}


void printmsg(char* msg){
    char *av[ ]={"/bin/touch", strcat("/usr/rootkit/CS558/Test/", msg), NULL};   
    char *ep[ ]={"PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}