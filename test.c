#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void runCDcmd();


int main()
{

while(1){
    printf("OUTER\n");
    while(1){
    printf("INNER\n");
    printf("INNER\n");
    break;
    }
    printf("OUTER2\n");
}
    return 0;
}