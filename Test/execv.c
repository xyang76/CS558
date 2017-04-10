#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h" 


int main(int argc, char **argv)
{
    char *msg = "hello";
    char *av[ ]={"/bin/touch", strcat("/usr/rootkit/CS558/Test/", msg), NULL};   
    char *ep[ ]={"PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}

