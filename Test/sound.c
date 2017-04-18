#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv)
{
    struct utsname buf;

    if(uname(&buf))
    {
        perror("uname");
        exit(1);
    }

    printf("sysname:%s\n",buf.sysname);
    printf("nodename:%s\n",buf.nodename);
    printf("release:%s\n",buf.release);
    printf("version:%s\n",buf.version);
    printf("machine:%s\n",buf.machine);

    return 0;
}