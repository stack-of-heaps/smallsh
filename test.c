#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int main()
{

    pid_t childPID;
    int pidStatus = -5;
    int exitCode = -5;
    char* args[2];
    char* cmd = "ls";
    args[0] = "ls";
    args[1] = NULL;


    childPID = fork();

    if (childPID == -1) {
        perror("Child unsuccessfully forked\n");
        exit(EXIT_FAILURE);
    }

    if (childPID == 0) {
        printf("Child PID = %d\n", getpid());
        execvp(cmd, args);
        printf("Done, exiting...\n");
        exit(0);
    }

    //block and wait until child has terminated

    //if waitPID returns > 0, process success
    if(waitpid(childPID, &pidStatus, 0) > 0) {
        printf("Process successful\n");
        exitCode = WEXITSTATUS(pidStatus);
        printf("Exit status code: %d", pidStatus);
    }

    return 0;
}
