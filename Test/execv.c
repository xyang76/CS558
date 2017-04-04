#include "stdio.h"
#include "stdlib.h"
#include "string.h"


int main(int argc, char **argv)
{
    char *av[ ]={"ls", "-al", "/etc/passwd", NULL};   
    char *ep[ ]={"PATH=/bin", NULL}   
    execve("/bin/ls", av, ep);   
}