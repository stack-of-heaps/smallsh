
/**************************************************************
 * NAME: KYLE KARTHAUSER
 * DATE: 5/25/19
 * COURSE: CS344-400
 * DESCRIPTION: "Small shell" program which runs normal bash commands,
 * runs processes in background, tracks all processes, and responds to 
 * special commands "status" and "exit."
 * ////////////////////////////////////////////////////////////*/

#include "smallsh.h"
//By default, we will run in bg
static int allowBackgroundFlag = 1;
static int backgroundFlag = 0;
static int childPIDstatus = 0;
static int bgExitStatus = 0;
size_t maxBuf = 2048;

/*************************************************************
 * DESCRIPTION: If ctrl + z is pressed in the main process, this function is called.
 * Toggles between allowBackgroundFlag = 1 and 0. 
 * If 1, background processes will be allowed to run.
 * If 0, background processes will not be allowed to run.
 * We are using write() instead of printf() for reentrant property
 * /////////////////////////////////////////////////////////*/
void watchSIGTSTP() {

    //reset our background flag, if set
	if (allowBackgroundFlag == 1) {
		char* alert = "Entering foreground-only mode (& is now ignored)\n";
		write(2, alert, 49);
		fflush(stdout);
		allowBackgroundFlag = 0;
	}

	else {
		char* alert = "Exiting foreground-only mode\n";
		write(2, alert, 29);
		fflush(stdout);
		allowBackgroundFlag = 1;
	}
}

/*************************************************************
 * DESCRIPTION: Handles requirement for our shell to print the last exit code of
 * the most recently exited/killed process. Called when user types 'status' in the
 * command line prompt.
 * /////////////////////////////////////////////////////////*/
void printLastStatus(int lastStatus) {
    int exitMacro;

    //If "normal" exit code
	if (WIFEXITED(lastStatus) != 0) {
        exitMacro = WIFEXITED(lastStatus);
		printf("Process exited with status code %d\n", exitMacro);
        fflush(stdout);
	} else {
        //Otherwise, terminated by a signal
        exitMacro = WIFSIGNALED(lastStatus);
		printf("Process terminated by signal %d\n", exitMacro);
        fflush(stdout);
	}
}

/*************************************************************
 * DESCRIPTION: Simple helper function that allocates memory to our args struct,
 * which is a char** array that holds up to 200 char strings for each index value
 * of args[i].
 * /////////////////////////////////////////////////////////*/
void memsetArgs(char** argsArray) {
    int i;
    for (i = 0; i < 512; i++) {
        argsArray[i] = calloc(200, 1);
        strcpy(argsArray[i], " ");
    }
}

/*************************************************************
 * DESCRIPTION: Simple helper function that allocates memory to a given char* pointer
 * for the given length # of bytes. Sets those bytes to '\0'
 * /////////////////////////////////////////////////////////*/
void memsetThis(char* someStr, int length) {
    memset(someStr, '\0', length);
}

/*************************************************************
 * DESCRIPTION: After we have parsed input, this runs. It takes a char** args, which is an 
 * array of strings which contain our commands to be executed. The last value of the array
 * will always be "NULL" for execvp() to run properly.
 * It also takes readFile and writefile char* pointers. If they contains values, they are 
 * read, a file descriptor is created, and stdout/stdin is redirected to the file pointer
 * so that the given command will execute properly in the sub-process.
 * Our sigaction struct facilitates ctrl+c being able to kill the child process.
 * Run as background is 1 if our parse function has detected a & at the end of input
 * and if allowBackgroundFlag is 0.
 * /////////////////////////////////////////////////////////*/
int runProcess(char** args, char* readFile, char* writeFile, struct sigaction sa, int* exitStatus, int* bgProcs) {

    int readFd, writeFd;
    int writeFDflag = 0;
    int fdResult;
    int pidStatus = -5;
    pid_t exitCode = -5;
    pid_t childPID = -5;

        childPID = fork();

        switch(childPID) {
            case -1: 
                printf("Error forking process\n");
                exit(EXIT_FAILURE);
                break;

            case 0: 
                // Allow CTRL-C to kill the child process
                sa.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sa, NULL);

                //Begin execution section
                //1) Create input/output file descriptors if necessary
                //2) Execute commands found in args: execvp(args[0], args);
                if (strcmp(readFile, "") != 0) {
                    //printf("readfile\n");
                    readFd = open(readFile, O_RDONLY);
                    if (readFd == -1) {
                        printf("%s is a bad file. Unable to open.\n", readFile);
                        fflush(stdout);
                        return -1;
                    }
                    //Create filein file descriptor
                    //Redirect stdin
                    //Copy stdin so we can restore it
                    fdResult = dup2(readFd, 0);
                    if (fdResult == -1) {
                        printf("Error in input redirection.\n");
                        fflush(stdout);
                        return -1;
                    }
                    else {
                        fcntl(readFd, F_SETFD, FD_CLOEXEC);
                    }
                }

                if (strcmp(writeFile, "") != 0) {
                    //printf("writefile: %s\n", writeFile);
                    writeFd = open(writeFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (writeFd == -1) {
                        printf("%s is a bad file. Unable to open.\n", writeFile);
                        fflush(stdout);
                        return -1;
                    }
                    //Create fileout file descriptor
                    //Redirect stdout
                    //Copy stdout so we can restore it
                    writeFDflag = 1;
                    fdResult = dup2(writeFd, 1);
                    if (fdResult == -1) {
                        printf("Error in output redirection.\n");
                        fflush(stdout);
                        return -1;
                    }
                    else {
                        fcntl(writeFd, F_SETFD, FD_CLOEXEC);
                    }
                }
               
                //printf("before execvp\n");
                //printf("args[0]: %s\n", args[0]);

                int execResult = execvp(args[0], args);
                if (execResult == -1) {
                    printf("Error: bad command or file: %s\n", args[0]);
                    fflush(stdout);
                    return -1;
                }
                break;

                //If we aren't running in the background, wait "normally," blocking main process until child finishes
                default: if (allowBackgroundFlag == 0 || backgroundFlag == 0) {
                    // execute like normal, blocking main process
                    waitpid(childPID, exitStatus, 0);
                }
                //If we are running in the background, print the PID
                //And add this newest bgprocess to our array to keep track of it
                else if (allowBackgroundFlag == 1 && backgroundFlag == 1) {
                    int numProcs = bgProcs[0];
                    numProcs++;
                    bgProcs[0] = numProcs;
                    bgProcs[numProcs] = childPID;
                    printf("Background Process ID: %d\n", childPID);
                    fflush(stdout);
                }
                break;
        }

        //check if any of our background processes has finished, one by one
        if (bgProcs[0] > 0) {
            int idx;
            for (idx = 1; idx <= bgProcs[0]; idx++) {
                childPID = waitpid(bgProcs[idx], &exitStatus, WNOHANG);
                
                if (childPID > 0) {
                    printLastStatus(exitStatus);
                }
            }
        }
}

int getAndParse(char* buffer, char** args, char* readFile, char* writeFile, int mainPID) {
    int numArgs = 0;
    int cmdStatus = 0;
    memsetThis(buffer, 2048);
    memsetThis(readFile, 2048);
    memsetThis(writeFile, 2048);
    numArgs=0;
    while(1) {
        numArgs = 0;
        cmdStatus = 0;
        backgroundFlag = 0;
        memsetArgs(args);
        memsetThis(buffer, 2048);
        cmdStatus = getline(&buffer, &maxBuf, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';        //Remove \n character at end of input

        if (cmdStatus == -1) {
            clearerr(stdin);
            printf("Error in input. Please try again.\n");
            fflush(stdout);
            return -1;
        }
        //Check for # as first character or for no input. If present, this is a comment and we do nothing
        else if (buffer[0] == '#') {
            return -1;
        }
        //Check if only spaces entered. If so, return to getting input loop
        else if (buffer[0] == ' ') {
            return -1;
        } //If we get here, we've got some input to act on
        else if (buffer[0] == NULL) {
            return -1;
        }
        else {
            break;              //We've got meaningful input; continue to parse section
        }
    }

    //Start parsing command line input
    char* token = strtok(buffer, " ");

    int i;
    for (i = 0; token != NULL; i++) {
        //If we see this we need to trip the background flag
        //When we exit, we will run the passed-in command in the background
        if (strcmp(token, "&") == 0) {
            backgroundFlag = 1;
        }

        //string following < will be our writeFile name
        else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            strcpy(readFile, token);
        }

        //string following > will be our writeFile name
        else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            strcpy(writeFile, token);
        }

        //if we get here, we're looking at a command, an arg, or a path
        else {
            strcpy(args[numArgs], token);
            numArgs++;
        }
        
        //Examine next part of buffer
        token = strtok(NULL, " ");
        }

    //Smallsh does not natively convert $$ from gradingscript into current PID. Convert that here.
    int lengthToSign = 0;
    int strLength = 0;
    char* newString;
    char pidStr[30];
    memsetThis(pidStr, 30);
    sprintf(pidStr, "%d", mainPID);
    for (i = 0; i < numArgs; i++) {
        lengthToSign = 0;
        strLength = strlen(args[i]);
        lengthToSign = strcspn(args[i], "$");

        //If we find a $, then these will not be equal
        //We also need to make sure that we see $$ and not just $
        if (strLength != lengthToSign && args[i][lengthToSign + 1] == '$') {
            char newString[128];
            memsetThis(newString, 128);
            strncpy(newString, args[i], lengthToSign);
            //printf("Original args: %s\n", args[i]);
            //printf("Truncated args: %s\n", newString);
            strcat(newString, pidStr);
            //printf("Finished product: %s\n", newString);
            memsetThis(args[i], 200);           //reset args[i] before replacing with new string, which includes expanded $$
            strcpy(args[i], newString);
        }
    }

    //Make sure we add NULL after the final arg in our args[] array for execvp()
    args[numArgs] = NULL;

    return 0;
    }

int main() {
    int backgroundFlag = 0;
    int exitStatus = 0;
    int mainPID = getpid();

    //Allocate memory
    char* cmdLineBuffer = NULL;         //Holds cmd line input
    cmdLineBuffer = calloc(maxBuf, 1);
    char** args;                        //Holds args which eventually execute with execvp()
    args = calloc(512, sizeof(char*));

    int bgProcs[512] = {0};                      //Holds PIDs of background processes
                                        //bgProcs[0] will ALWAYS hold # of bg processes at a given time
    int cmdStatus = -5;
    char readFile[2048];
    char writeFile[2048];

    int readFd;                         //read and write file descriptors
    int writeFd;
    int fdResult;                      //holds errors in creating file descriptors

    int parseFlag = 0;
    int backupStdout;
    int backupStdin;

    // Signal Handler to ignore ^C
	struct sigaction sa_sigint = {0};
	sa_sigint.sa_handler = SIG_IGN;
	sigfillset(&sa_sigint.sa_mask);
	sa_sigint.sa_flags = 0;
	sigaction(SIGINT, &sa_sigint, NULL);

    // Signal Handler to redirect ^Z to watchSIGTSTP()
	struct sigaction sa_sigtstp = {0};
	sa_sigtstp.sa_handler = watchSIGTSTP;
	sigfillset(&sa_sigtstp.sa_mask);
	sa_sigtstp.sa_flags = 0;
	sigaction(SIGTSTP, &sa_sigtstp, NULL);

    while(1) {
        while(1) {
            exitStatus = 0;
            readFd = 0;
            writeFd = 0;
            printf("%s", ":");

            int cmdStatus = getAndParse(cmdLineBuffer, args, readFile, writeFile, mainPID);

            if (cmdStatus == -1) {
                continue;
            }

            else {
                break;
            }
        }

        //We need to execute any directory related commands here in the main proc
        //If "cd", handle it here
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                char* path = getenv("HOME");
                chdir(path);
                continue;
            }
            else {
                chdir(args[1]);
                continue;
            }
        }

        //Special command: "quit"
        //Kill all child processes, then exit(0)
        else if (strcmp(args[0], "quit") == 0) {
            exit(0);
        }

        //Special command: "status"
        //Return last exit status code
        else if (strcmp(args[0], "status") == 0) {
            printLastStatus(exitStatus);
            fflush(stdout);
            continue;
        }

        //All other commands:
        else {
            runProcess(args, readFile, writeFile, sa_sigint, &exitStatus, bgProcs);
        }
    }
    fflush(stdout);
    return 0;
}