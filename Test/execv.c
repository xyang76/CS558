#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h" 

int printmsg(char*msg);

int main(int argc, char **argv)
{
    char *msg = "Hello world!";
    printmsg(msg);
}


int printmsg(char* msg){
    char *av[ ]={"/bin/echo", strcat(msg, ">/usr/rootkit/CS558/Test/msg.txt") , NULL};   
    char *ep[ ]={"PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}