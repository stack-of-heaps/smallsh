#include "smallsh.h"

void memsetArgs(char** argsArray) {
    int i;
    for (i = 0; i < 512; i++) {
        argsArray[i] = calloc(20, 1);
    }
}

void memFreeArgs(char** argsArray) {
    int i;
    for (i = 0; i < 512; i++) {
        free(argsArray[i]);
    }
}

//Using execvp, runs process specified by cmd along with everything in args[]
//Uses a fork to run process, then blocks with waitpid()
//Returns exit code
int runProcess(char* cmd, char** args) {

    pid_t childPID;
    int pidStatus = -5;
    int exitCode = -5;

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
        printf("Exit status code: %d\n", pidStatus);
        return exitCode;
    }
}

/*
wait() -- blocks until ANY child process terminates; returns PID of termindated child
waitpid()   -- blocks until specific child process terminates; OR, passed special flag, reports immediately on status of child process
                  
         1) block parent until any child process terminates --> childPID = wait(&childExit)
         2) block parent until specified child temrinates --> waitPID(childPID_intent, &childExitMethod, 0)
         3) check if any process has completed, return immediately with 0 if none have
                  ---> childPID = waitpid(-1, &childExit, WNOHANG)
         4) check if specific process has completed, return immediately with 0 if it has NOT 
                  ---> childPID_actual = waitpid(childPID_intent, &childExit, WNOHANG)

          if a process terminates normally, then WIFEXITED macro returns non-zero:
              if (WIFEXITED(child) !=0)  ---->   printf("the process exited normally\n");

TO GET ACTUAL EXIT STATUS, MUST EXAMINE FLAG: int exitStatus = WEXITSTATUS(child);
          If exited successfully, WIFEXITED macro returns non-zero

SIGNAL TERMINATION
          --If process terminated by signal, WIFSIGNALED macro returns non-zero
              if (WIFSIGNALED(cihldExitMethod!= 0)) -> printf("Process terminated by a signal")

EXEC            Replaces currently running program with NEW ONE specified by us
                  --Do not return. DESTROY currently running prog
                  --Args passed to exec() become COMMAND-LINE args that show up as argc/argv in C, and $1/$2 in bash
       int execL(char *path, char*arg1, ... char* argn); 
                  --Executes program specified by path, and gives command line args
       int execV(char *path, char* argv[]);
                  --Executes program at path, and gives command line args indicated by pointers in argv
       getcwd() gets current dir
       chdir() sets current dir

TO USE EXEC, PAIR IT WITH *FORK*
       1) fork main proc
             fork() -- creates child process --> spawnPID = fork()
       2) use exec on child
       3) keep track of PID and kill process when finished / no longer needed

EXECL("ls", "ls", "-a", NULL); // NULL says "end of args" to function
EXECVP
    char* args[3] = {"ls, "-a", NULL};
    execvp(args[0], args);
    spawnPID = fork();

EXIT()
    **atExit() registers function to RUN BEFORE EXIT()
    --Calls all functions registered by atexit()
    --Flushes all STDIO output
    --Removes files created by tmpfile()
    --Calls _exit (cleans up everything, like return 0 from main() function)

ENV VARIABLES
 */         

int main() {

    char* cmdLineBuffer = NULL;     //Holds cmd line input
    size_t maxBuf = 2048;           //Max input size for cmdline
    char* prompt = ":";             //Terminal prompt
    char* tok;
    char** args;
    int cmdStatus = -5;
    char* cmd;
    char* cmdArgs;
    char* readCmd;
    char* writeCmd;
    int readCharDist = 0;
    int writeCharDist = 0;
    char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int cmdLength = 0;

    //Allocate memory
    args = calloc(512, sizeof(char*));
    cmdLineBuffer = calloc(maxBuf, 1);

    while(1) {
        while(1) {
            memsetArgs(args);
            memset(cmdLineBuffer, '\0', 2048);
            printf("%s", prompt);
            fflush(stdout);
            cmdStatus = getline(&cmdLineBuffer, &maxBuf, stdin);

            if (cmdStatus == -1) {
                clearerr(stdin);
            }
            //Check for # as first character or for no input. If present, this is a comment and we do nothing
            else if (cmdLineBuffer[0] == '#') {
                continue;
            }
            //Check if only spaces entered. If so, quit and return to getting input loop
            else if (strlen(cmdLineBuffer) == strcspn(cmdLineBuffer, alphabet)) {
                continue;
            }
            else {
                break;
            }
        }

       //Remove \n from input
       cmdLineBuffer[strcspn(cmdLineBuffer, "\n")] = '\0';
       
       //First "word" will be the program to call
       cmd = calloc(100, 1);
       sscanf(cmdLineBuffer, "%s", cmd);

       //We test to see if this is the only thing entered and skip all the rest; go straight to exec()
       if (strlen(cmd) == strlen(cmdLineBuffer)) {
           strcpy(args[0], cmd);
           args[1] = NULL;
           runProcess(cmd, args);
       }
       else {

       //Get subsequent args:
       //0) Save position of command; use it to get position of following args
       //1) Check for < char
       //   If present, note position
       //2) Check for > char
       //   If present, note position
       //3) If both < & >, see which is first
       //4) 
       
       //First we'll check for read/write chars
       char* readCharLoc = strchr(cmdLineBuffer, '<');
       char* writeCharLoc = strchr(cmdLineBuffer, '>');

       //Check for presence of & char
       char* ampersand = strchr(cmdLineBuffer, '&');

       //If both are present, we need to see which is first
       if (readCharLoc && writeCharLoc) {
           int firstCharDist = 0;
           readCharDist = 0;
           writeCharDist = 0;
           printf("Both present\n");

           readCharDist = strcspn(cmdLineBuffer, "<");
           writeCharDist =  strcspn(cmdLineBuffer, ">");

           printf("<: %d\n", readCharDist);
           printf(">: %d\n", writeCharDist);

           //Get the smallest distance; assign to firstCharDist
           if (readCharDist < writeCharDist) {
               firstCharDist = readCharDist;
           }
           else {
               firstCharDist = writeCharDist;
           
        char firstHalf[2048];
        char secondHalf[2048];
        memcpy(firstHalf, "\0", 2048);
        memcpy(secondHalf, "\0", 2048);
           }
       }

       //else if ONLY ONE READ/WRITE CHAR IS PRESENT....)

        //Otherwise, we've got one cmd with one or more args to vacuum up and pass on
        else {
        char tempArg[512];
        memset(tempArg, '\0', 512);
        int numArgs = 0;
        char* strItr = cmdLineBuffer;      //Start sscanf looking PAST the first word we've already inserted into arg[0];
        while (sscanf(strItr, "%s", tempArg) != EOF) {
                strcpy(args[numArgs], tempArg);
                strItr += strlen(tempArg) + 1;  //Start scanning for next arg AFTER this one + space
                printf("Arg: %s\n", tempArg);
                memset(tempArg, '\0', 500);
                numArgs++;
                }

                args[numArgs] = NULL;
                runProcess(cmd, args);
                }
            }
    }
    //Free all of our dynamically allocated memory
    //free(cmdLineBuffer);
    //free(cmd);
    //memFreeArgs(args);
    //free(args);
    return 0;

}

/*
// READ IN ADN WRITE TO < > commands (wc < junk > junk2
//if last char is &, perform in background
//do NOTHING when # is first char
//do NOTHING when input blank
//sleep
//kill
//pwd
//date

The general syntax of a command line is:

command [arg1 arg2 ...] [< input_file] [> output_file] [&]

where [ ] are optional
*/

//Where there are no < > chars...
void parseCommandLine(char* cmdInput);

//Where there are < > chars and we need to stop reading at that point and deal with them separately
void parseCommandLineRedirects(char* cmdInput, int n);

//invoked when user types "exit"
void killAllProcesses();

//"cd" with no args -> dir specified by HOME environment variable
//otherwise takes 1 command, absolute or relative path
//does not change directory of parent process; i.e., start program in users/bin/a, program exits still in users/bin/a
//use chdir()
int cd(char* path);


int quit();

//if the last char of input is &, it is to be performed in the background
void backgroundProcess(char* process);


