#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void runCDcmd();


int main()
{

    pid_t childPID;
    int pidStatus = -5;
    int exitCode = -5;
    char path[256];

    chdir("..");


    chdir("adventure");

    getcwd(path, sizeof(path));
    printf("%s", path);

    return 0;
}