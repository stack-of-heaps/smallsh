#ifndef SMALLSH
#define SMALLSH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

void memsetArgs(char** argsArray);
int parseCmdLine(char* buffer, char** args, char* cmd);
int getAndParse(char* buffer, char** args, char* readFile, char* writeFile, int* backgroundFlag);
void memsetThis(char* someStr, int length);
void printLastStatus();
int runProcess(char** args, char* readFile, char* writeFile, struct sigaction sa, int runAsBackground);
void watchSIGTSTP();

#endif