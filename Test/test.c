#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MONITOR_RESULT "monitoroutput.txttmp"

int main(int argc, char **arg0)
{
    char *m = "'hello'>>/usr/a.txt";
    char *argv[] = { "bin/echo", m, NULL};
    char *envp[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};
    execve(argv[0], argv, envp);
}