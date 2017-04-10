#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h" 

int printmsg(char*msg);

int main(int argc, char **argv)
{
    char *msg = "Hello!";
    printmsg(msg);
}


int printmsg(char* msg){
    char *m = strcat(strcat("'", msg), "'>/usr/rootkit/CS558/Test/TestTouch");
    char *av[ ]={"/bin/echo", m , NULL};   
    char *ep[ ]={"PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}