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
    char m[256] = "/usr/rootkit/CS558/Test/";
    strcat(m, msg);
    char *av[ ]={"/bin/touch", m , NULL};   
    char *ep[ ]={"HOME=/","TERM=linux","PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}