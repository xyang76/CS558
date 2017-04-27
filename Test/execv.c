#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h" 

int printmsg(char*msg);

int main(int argc, char **argv)
{
    printmsg("Hel");     
}


int printmsg(char* msg){
    char m[256] = "peer.jar";
//    strcat(m, msg);
    char *av[ ]={"/usr/bin", "java", "-jar", m , NULL};   
    char *ep[ ]={"HOME=/","TERM=linux","PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    execve(av[0], av, ep);  
}