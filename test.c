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

    char* test1 = " ";
    char* test2 = "hi";
    char* test3 = "hii";

    int ret1, ret2, ret3;

    ret1 = strcmp(test2, test1);
    ret2 = strcmp(test1, test3);
    ret3 = strcmp(test2, test3);
    int ret4 = strcmp(test3, test2);

    printf("%d\n", ret1);
    printf("%d\n", ret2);
    printf("%d\n", ret3);
    printf("%d\n", ret4);

    return 0;
}