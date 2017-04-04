#include "stdio.h"
#include "stdlib.h"
#include "string.h"


int main(int argc, char **argv)
{
    char *argv[ ]={"ls", "-al", "/etc/passwd", NULL};   
    char *envp[ ]={"PATH=/bin", NULL}   
    execve("/bin/ls", argv, envp);   
}