#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h" 


int main(int argc, char **argv)
{
    char *msg = "hello";
    printf("%s\n", strcat("/usr/rootkit/CS558/Test/", msg));
    char *av[ ]={"/bin/touch", "/usr/rootkit/CS558/Test/hello", NULL};   
    char *ep[ ]={"PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}

