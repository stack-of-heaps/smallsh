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

void memsetArgs(char** argsArray);
void memFreeArgs(char** argsArray);
int parseCmdLine(char* buffer, char** args, char* cmd);
int parseAndRead(char* buffer, char** args, char* cmd);
int parseAndWrite(char* buffer, char** args, char* cmd);
int getFileToRead(char* buffer, char* filename);
int getFileToWrite(char* buffer, char* filename);
void memsetThis(char* someStr, int length);


void parseCommandLineRedirects(char* cmdInput, int n);
int runProcess(char* cmd, char** args);
int runPrompt();

//Need ints for all of these because we must return 
//success (0) or failure (1)
int getStatus();
void cd();
int quit();

#endif