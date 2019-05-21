#include "smallsh.h"
static int lastStatus = 0;

void memsetArgs(char** argsArray) {
    int i;
    for (i = 0; i < 512; i++) {
        argsArray[i] = calloc(20, 1);
    }
}


void memsetThis(char* someStr, int length) {
    memset(someStr, '\0', length);
}

int main() {
    char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int backgroundFlag = 0;

    //Allocate memory

    char* cmdLineBuffer = NULL;     //Holds cmd line input
    size_t maxBuf = 2048;           //Max input size for cmdline
    char* prompt = ":";             //Terminal prompt
    char** args;
    int cmdStatus = -5;
    char* cmd;
    char* cmdArgs;
    int cmdLength = 0;
    char readFile[512];
    char writeFile[512];
    int numArgs;
    int specialFlag;




    while(1) {
        getAndParse(cmdLineBuffer, args, readFile, writeFile);

        if (!strcmp(args[0], "cd")) {
            if (strcmp(args[1], "") {
               strcpy(args[1], getenv("HOME"));
               chdir(args[1]);
           }
           else {
               chdir(args[1]);
           }
        }


    args = calloc(512, sizeof(char*));
    cmdLineBuffer = calloc(maxBuf, 1);
    memsetThis(cmdLineBuffer, 2048);
    memsetThis(readFile, 512);
    memsetThis(writeFile, 512);
    numArgs=0;

        while(1) {
            memsetArgs(args);
            memsetThis(cmdLineBuffer, 2048);
            printf("%s", prompt);
            fflush(stdout);
            cmdStatus = getline(&cmdLineBuffer, &maxBuf, stdin);
            cmdLineBuffer[strcspn(cmdLineBuffer, "\n")] = '\0';

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
            } //If we get here, we've got some input to act on
            else {
                break;
            }
        }

    char* token = strtok(cmdLineBuffer, " ");

    int i;
    for (i = 0; token; i++) {
        if (!strcmp(token, "&")) {
            backgroundFlag = 1;
        }

        else if (!strcmp(token, "<")) {
            token = strtok(NULL, " ");
            strcpy(readFile, token);
        }
        
        else if (!strcmp(token, ">")) {
            token = strtok(NULL, " ");
            strcpy(writeFile, token);
        }

        else {
            strcpy(args[numArgs], token);
            numArgs++;
        }

        token = strtok(NULL, " ");
        }

        for (i = 0; i < numArgs; i++) {
            printf("ARG[%d]: %s\n", i, args[i]);
        }

if (strcmp(readFile, "")) {
				// open it
                /*
				input = open(inputName, O_RDONLY);
				if (input == -1) {
					perror("Unable to open input file\n");
					exit(1);
				}
				// assign it
				result = dup2(input, 0);
				if (result == -1) {
					perror("Unable to assign input file\n");
					exit(2);
				}
				// trigger its close
				fcntl(input, F_SETFD, FD_CLOEXEC);
                */
               printf("readfile: %s\n", readFile);
			}

if (strcmp(writeFile, "")) {
    printf("rritefile: %s\n", writeFile);

    }

}
return 0;
}
