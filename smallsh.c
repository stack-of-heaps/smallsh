#include "smallsh.h"
static int lastStatus = 0;

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

int parseCmdLine(char* buffer, char** args, char* cmd) {

    char tempArg[512];
    memset(tempArg, '\0', 512);
    int numArgs = 0;
    char* strItr = buffer;      //Start sscanf looking PAST the first word we've already inserted into arg[0];
    while (sscanf(strItr, "%[.0-9A-Za-z<>*""''&-]", tempArg) != EOF) {

        //If we aren't reading in a redirect char, read input string as normal arg/cmd
        if (strcmp(tempArg, "<") != 0 && strcmp(tempArg, ">") != 0) {
            printf("TEMPARG: '%s'\n", tempArg);
            strcpy(args[numArgs], tempArg);
            strItr += strlen(tempArg) + 1;  //Start scanning for next arg AFTER this one + space
            printf("Arg: %s\n", tempArg);
            memset(tempArg, '\0', 500);
            numArgs++;
        }
        else {
            //Write redirect char
            //1) Grab following arg as filename
            //2) Create file descriptor; backup STDOUT
            //3) Finalize args array by appending NULL
            //4) Redirect stdout to fileptr from 2
            //5) Runproc
            if (strcmp(tempArg, ">") == 0) {
                printf("IN WRITING MODE\n");

                char fileName[512];
                memset(fileName, '\0', 512);
                strItr += strlen("> ");
                sscanf(strItr, "%s", fileName);
                printf("filename: '%s'\n", fileName);

                args[numArgs] = NULL;
                
                int fd;
                fd = open(fileName, O_CREAT | O_WRONLY | O_APPEND, 0600); 

                int saved_stdout = dup(1);
                dup2(fd, 1);

                lastStatus = runProcess(cmd, args);

                dup2(saved_stdout, 1);
                close(saved_stdout);
                return -1;
            }
        }
    }

    args[numArgs] = NULL;

    if (strcmp(cmd, "cd") == 0) {
        return 1;
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
        exit(EXIT_FAILURE);
    }

    if (childPID == 0) {
        execvp(cmd, args);
        exit(0);
    }

    //block and wait until child has terminated
    //if waitPID returns > 0, process success
    if(waitpid(childPID, &pidStatus, 0) > 0) {
        exitCode = WEXITSTATUS(pidStatus);
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
EXIT()
    **atExit() registers function to RUN BEFORE EXIT()
    --Calls all functions registered by atexit()
    --Flushes all STDIO output
    --Removes files created by tmpfile()
    --Calls _exit (cleans up everything, like return 0 from main() function)
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
    int lastExitStatus = 0;

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
       
       //First "word" will be the program to call
       cmdLineBuffer[strcspn(cmdLineBuffer, "\n")] = '\0';
       cmd = calloc(100, 1);
       sscanf(cmdLineBuffer, "%s", cmd);

        //Here we check the cmd passed in. If it's cd, exit, or status, they are special commands
        //and are handled individually.
        //If there is only one command and it doens't match the above, execute immediately
        //If there are multiple commands, we pass into the "else" branch
       if (strlen(cmd) == strlen(cmdLineBuffer)) {
           if (strcmp(cmd, "cd") == 0) {
               char* path = getenv("HOME");
               chdir(path);
           }
           else if (strcmp(cmd, "status") == 0) {
               printf("%d\n", lastStatus);
           }
           else {
           strcpy(args[0], cmd);
           args[1] = NULL;
           lastStatus = runProcess(cmd, args);
           fflush(stdout);
           }
       }
       //else if ONLY ONE READ/WRITE CHAR IS PRESENT....)

        //Otherwise, we've got one cmd with one or more args to vacuum up and pass on
        //Parse, run, loop back
        else {
        int retFlag = 0;

        //parseCmdLine will look for cd + path arg here 
        //cd = 1; 
        retFlag = parseCmdLine(cmdLineBuffer, args, cmd);
        printf("retflag: %d\n", retFlag);

        //execute chdir with path in args[1] 
        if (retFlag == 1) {
            char* path = args[1];
            chdir(path);
        }
        //-1 means we've already parsed and executed a process
        //which uses dup2 on stdout, executes the cmd, then restores stdout
        //go back to smallsh prompt
        else if (retFlag == -1) {
        //    continue;
        }
        else {
            lastStatus = runProcess(cmd, args);
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

//Changes current program's PWD to $HOME env variable path unless a path is passed in
void cd(char* pathIn) {

    if (pathIn == NULL) {
        char* path;
        path = getenv("HOME");
        chdir(path);
    }
    else {
        chdir(pathIn);
    }
}

int quit();


//if the last char of input is &, it is to be performed in the background
void backgroundProcess(char* process);


