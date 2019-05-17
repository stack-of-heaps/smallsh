#ifndef SMALLSH
#define SMALLSH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void memsetArgs(char** argsArray);
void memFreeArgs(char** argsArray);
void parseCommandLine(char* cmdInput);
void parseCommandLineRedirects(char* cmdInput, int n);
int runProcess(char* cmd, char** args);

//Need ints for all of these because we must return 
//success (0) or failure (1)
int getStatus();
int cd();
int quit();

#endif
