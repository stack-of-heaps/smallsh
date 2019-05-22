#include "smallsh.h"
static int lastStatus = 0;
static int allowBackgroundFlag = 0;
static int childPIDstatus = 0;
size_t maxBuf = 2048;

/***************************************************************
 * 			void catchSIGTSTP(int)
 *
 * When SIGTSTP is called, toggle the allowBackground boolean.
 *
 * INPUT:
 * 	int signo		Required, according to the homework. Isn't used.
 *
***************************************************************/
//we use write() instead of printf() for reentrant property
void watchSIGTSTP() {

    //reset our background flag, if set
	if (allowBackgroundFlag == 1) {
		char* alert = "Entering foreground-only mode (& is now ignored)\n";
		write(1, alert, 49);
		fflush(stdout);
		allowBackgroundFlag = 0;
	}

    //
	else {
		char* alert = "Exiting foreground-only mode\n";
		write(1, alert, 29);
		fflush(stdout);
		allowBackgroundFlag = 1;
	}
}


void memsetArgs(char** argsArray) {
    int i;
    for (i = 0; i < 512; i++) {
        argsArray[i] = calloc(200, 1);
        strcpy(argsArray[i], " ");
    }
}

void memsetThis(char* someStr, int length) {
    memset(someStr, '\0', length);
}

int runProcess(char** args, char* readFile, char* writeFile, struct sigaction sa, int runAsBackground ) {

    int readFd, writeFd;
    int fdResult;
    int backupStdin, backupStdout;
    pid_t childPID;
    int pidStatus = -5;
    int exitCode = -5;

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
                if (strcmp(readFile, "") == 1) {
                    printf("readfile\n");
                    readFd = open(readFile, O_RDONLY);
                    if (readFd == -1) {
                        printf("%s is a bad file. Unable to open.\n", readFile);
                        lastStatus = -1;
                        break;
                    }
                    //Create filein file descriptor
                    //Redirect stdin
                    //Copy stdin so we can restore it
                    backupStdin = dup(1);
                    fdResult = dup2(readFd, 0);
                    if (fdResult == -1) {
                        printf("Error in input redirection.\n");
                        lastStatus = fdResult;
                        break;
                    }
                }

                if (strcmp(writeFile, "") == 1) {
                    printf("rritefile: %s\n", writeFile);
                    writeFd = open(writeFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (writeFd == -1) {
                        printf("%s is a bad file. Unable to open.\n", writeFile);
                        lastStatus = -1;
                    }
                    //Create fileout file descriptor
                    //Redirect stdout
                    //Copy stdout so we can restore it
                    backupStdout = dup(0);
                    fdResult = dup2(writeFd, 1);
                    if (fdResult == -1) {
                        printf("Error in output redirection.\n");
                        lastStatus = fdResult;
                        break;
                    }
                }
               
                //printf("before execvp\n");
                //printf("args[0]: %s\n", args[0]);

                execvp(args[0], args);
                dup2(backupStdout, 1);
                close(backupStdout);
                dup2(backupStdin, 0);
                close(backupStdin);
                break;



            default:;

            if (allowBackgroundFlag && runAsBackground == 1) {
				pid_t actualPid = waitpid(childPID, childPIDstatus, WNOHANG);
				printf("Background PID: %d\n", childPID);
				fflush(stdout);
			}
			// Otherwise execute it like normal
			else {
				pid_t actualPid = waitpid(childPID, childPIDstatus, 0);
			}

		// Check for terminated background processes!	
		while ((childPID = waitpid(-1, childPIDstatus, WNOHANG)) > 0) {
			printf("Child Process %d terminated\n", childPID);
			//printExitStatus(*childExitStatus);
			fflush(stdout);

                }

                //block and wait until child has terminated
                //if waitPID returns > 0, process success
                if(waitpid(childPID, &pidStatus, 0) > 0) {
                    lastStatus = WEXITSTATUS(pidStatus);
                }

    }
}


int getAndParse(char* buffer, char** args, char* readFile, char* writeFile, int* backgroundFlag) {
    int numArgs = 0;
    int cmdStatus = 0;

    memsetThis(buffer, 2048);
    memsetThis(readFile, 512);
    memsetThis(writeFile, 512);
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

    char* token = strtok(buffer, " ");

    int i;
    for (i = 0; token; i++) {
        //If we see this we need to trip the background flag
        //When we exit, we will run the passed-in command in the background
        if (!strcmp(token, "&")) {
            backgroundFlag = 1;
        }

        //string following < will be our writeFile name
        else if (!strcmp(token, "<")) {
            token = strtok(NULL, " ");
            strcpy(readFile, token);
        }

        //string following > will be our writeFile name
        else if (!strcmp(token, ">")) {
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

    for (i = 0; i < numArgs; i++) {
        printf("ARG[%d]: %s\n", i, args[i]);
        }

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
    char readFile[512];
    char writeFile[512];

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

            parseFlag = getAndParse(cmdLineBuffer, args, readFile, writeFile, &backgroundFlag);

            if (parseFlag == -1) {
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
                printf("%d\n", lastStatus);
                fflush(stdout);
                continue;
            }

            //All other commands:
            else {
                runProcess(args, readFile, writeFile, sa_sigint, runInBackground);
            }
        }
    fflush(stdout);
    return 0;
}


