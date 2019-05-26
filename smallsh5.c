/**************************************************************
 * NAME: KYLE KARTHAUSER
 * DATE: 5/25/19
 * COURSE: CS344-400
 * DESCRIPTION: "Small shell" program which runs normal bash commands,
 * runs processes in background, tracks all processes, and responds to 
 * special commands "status" and "exit."
 * ////////////////////////////////////////////////////////////*/

#include "smallsh.h"
static int lastStatus = 0;
static int allowBackgroundFlag = 0;
static int childPIDstatus = 0;
static int bgExitStatus = 0;
size_t maxBuf = 2048;

/*************************************************************
 * DESCRIPTION: If ctrl + z is pressed in the main process, this function is called.
 * Toggles between allowBackgroundFlag = 1 and 0. 
 * If 0, background processes will be allowed to run.
 * If 1, background processes will not be allowed to run.
 * We are using write() instead of printf() for reentrant property
 * /////////////////////////////////////////////////////////*/
void watchSIGTSTP() {

    //reset our background flag, if set
	if (allowBackgroundFlag == 1) {
		char* alert = "Entering foreground-only mode (& is now ignored)\n";
		write(1, alert, 49);
		fflush(stdout);
		allowBackgroundFlag = 0;
	}

	else {
		char* alert = "Exiting foreground-only mode\n";
		write(1, alert, 29);
		fflush(stdout);
		allowBackgroundFlag = 1;
	}
}

/*************************************************************
 * DESCRIPTION: Handles requirement for our shell to print the last exit code of
 * the most recently exited/killed process. Called when user types 'status' in the
 * command line prompt.
 * /////////////////////////////////////////////////////////*/
void printLastStatus() {

    //If "normal" exit code
	if (WIFEXITED(lastStatus)) {
		printf("Last exit status code: %d\n", WEXITSTATUS(lastStatus));
	} else {
        //Otherwise, terminated by a signal
		printf("Last exit signal code:  %d\n", WTERMSIG(lastStatus));
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
int runProcess(char** args, char* readFile, char* writeFile, struct sigaction sa, int runAsBackground ) {

    int readFd, writeFd;
    int fdResult;
    int backupStdin = dup(1);       //In case we redirect input/output, backup here
    int backupStdout = dup(0);
    int pidStatus = -5;
    pid_t exitCode = -5;
    pid_t bgPID = -5;
    pid_t childPID = -5;
    int retTest = strcmp(readFile, "");

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
                        lastStatus = -1;
                        break;
                    }
                    //Create filein file descriptor
                    //Redirect stdin
                    //Copy stdin so we can restore it
                    fdResult = dup2(readFd, 0);
                    if (fdResult == -1) {
                        printf("Error in input redirection.\n");
                        lastStatus = fdResult;
                        break;
                    }
                }

                if (strcmp(writeFile, "") != 0) {
                    //printf("writefile: %s\n", writeFile);
                    writeFd = open(writeFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (writeFd == -1) {
                        printf("%s is a bad file. Unable to open.\n", writeFile);
                        lastStatus = -1;
                    }
                    //Create fileout file descriptor
                    //Redirect stdout
                    //Copy stdout so we can restore it
                    fdResult = dup2(writeFd, 1);
                    if (fdResult == -1) {
                        printf("Error in output redirection.\n");
                        lastStatus = fdResult;
                        break;
                    }
                }
               
                //printf("before execvp\n");
                //printf("args[0]: %s\n", args[0]);

                lastStatus = execvp(args[0], args);
                dup2(backupStdout, 1);
                close(backupStdout);
                dup2(backupStdin, 0);
                close(backupStdin);

                break;
        }

        //If we haven't disabled run-in-background,
        //and & was found in cmdline, run as background process
        if (allowBackgroundFlag && runAsBackground == 1) {
            pid_t childPID = waitpid(childPID, lastStatus, WNOHANG);
            printf("Background PID: %d\n", childPID);
            fflush(stdout);
        }
        // Otherwise execute it like normal
        else {
            lastStatus = waitpid(childPID, lastStatus, 0);
            fflush(stdout);
        }

        // Check for terminated background processes
        //Option B from Program 3 description: Check periodically if
        //any child processes have terminated via signal handler.
        while ((childPID = waitpid(-1, lastStatus, WNOHANG)) > 0) {
            printf("Child Process %d terminated\n", childPID);
            //getBGExitStatus(bgExitStatus);
            fflush(stdout);
        }

        //Finally, block out main process until child has terminated
        if(waitpid(childPID, lastStatus, 0) > 0) {
            lastStatus = WEXITSTATUS(pidStatus);
        }
}

int getAndParse(char* buffer, char** args, char* readFile, char* writeFile, int* backgroundFlag) {
    int numArgs = 0;
    int cmdStatus = 0;
    memsetThis(buffer, 2048);
    memsetThis(readFile, 2048);
    memsetThis(writeFile, 2048);
    numArgs=0;
    while(1) {
        numArgs = 0;
        cmdStatus = 0;
        memsetArgs(args);
        memsetThis(buffer, 2048);
        printf("%s", ":");
        fflush(stdout);
        cmdStatus = getline(&buffer, &maxBuf, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';        //Remove \n character at end of input

        if (cmdStatus == -1) {
            clearerr(stdin);
            printf("Error in input. Please try again.\n");
            fflush(stdout);
            return cmdStatus;
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

    //Make sure we add NULL after the final arg in our args[] array for execvp()

    args[numArgs] = NULL;

    return 0;
    }

int main() {
    int backgroundFlag = 0;

    //Allocate memory
    char* cmdLineBuffer = NULL;         //Holds cmd line input
    cmdLineBuffer = calloc(maxBuf, 1);
    char** args;                        //Holds args which eventually execute with execvp()
    args = calloc(512, sizeof(char*));

    int cmdStatus = -5;
    char readFile[2048];
    char writeFile[2048];

    int readFd;                         //read and write file descriptors
    int writeFd;
    int fdResult;                      //holds errors in creating file descriptors

    int parseFlag = 0;
    int backupStdout;
    int backupStdin;
    int runInBackground = 0;

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
            runInBackground = 0;
            readFd = 0;
            writeFd = 0;

            lastStatus = getAndParse(cmdLineBuffer, args, readFile, writeFile, &backgroundFlag);

            if (lastStatus == -1) {
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
            printLastStatus();
            fflush(stdout);
            continue;
        }

        //All other commands:
        else {
            lastStatus = runProcess(args, readFile, writeFile, sa_sigint, runInBackground);
        }
    }
    fflush(stdout);
    return 0;
}