#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MONITOR_RESULT "monitoroutput.txttmp"

int main(int argc, char **arg0)
{
    char *m = "Hello world!";
    char *argv[] = { "/usr/CS558/Demo/./monitor", m, NULL};
    char *envp[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};
    execve(argv[0], argv, envp);
}