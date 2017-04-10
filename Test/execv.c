#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h" 

int printmsg(char*msg);

int main(int argc, char **argv)
{
    printmsg("Hello");     
}


int printmsg(char* msg){
    char m[256];
    m = strcat(strcat("'", msg), "'>/usr/rootkit/CS558/Test/msg.txt");
    char *av[ ]={"/bin/echo", m , NULL};   
    char *ep[ ]={"PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}