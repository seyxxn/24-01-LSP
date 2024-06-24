#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void helpAdd()
{
    printf("add <PATH> :　record path to staging area, path will tracking modification\n");
}

void helpRemove()
{
    printf("remove <PATH> : record path to staging area, path will not tracking modification\n");
}

void helpStatus()
{
    printf("status : show staging area status\n");
}

void helpCommit()
{
    printf("commit <NAME> : backup staging area with commit name\n");
}

void helpRevert()
{
    printf("revert <NAME> : recover commit version with commit name\n");
}

void helpLog(){
    printf("log : show commit log\n");
}

void helpHelp(){
    printf("help : show commands for program\n");
}

void helpExit(){
    printf("exit : exit program\n");
}

void help(char *command)
{
    if (!strcmp(command, "all"))
    {
        printf("Usage:\n");
        printf("    > "); helpAdd();
        printf("    > "); helpRemove();
        printf("    > "); helpStatus();
        printf("    > "); helpCommit();
        printf("    > "); helpRevert();
        printf("    > "); helpLog();
        printf("    > "); helpHelp();
        printf("    > "); helpExit();
        printf("\n");
    }
    else if (!strcmp(command, "add"))
    {
        printf("Usage: "); helpAdd(); printf("\n");
    }
    else if (!strcmp(command, "remove"))
    {
        printf("Usage: "); helpRemove(); printf("\n");
    }
    else if (!strcmp(command, "status"))
    {
        printf("Usage: "); helpStatus(); printf("\n");
    }
    else if (!strcmp(command, "commit"))
    {
        printf("Usage: "); helpCommit(); printf("\n");
    }
    else if (!strcmp(command, "revert"))
    {
        printf("Usage: "); helpRevert(); printf("\n");
    }
    else if (!strcmp(command, "log"))
    {
        printf("Usage: "); helpLog(); printf("\n");
    }
    else if (!strcmp(command, "help"))
    {
        printf("Usage: "); helpHelp(); printf("\n");
    }
    else if (!strcmp(command, "exit"))
    {
        printf("Usage: "); helpExit(); printf("\n");
    }
    else
    {
        // printf("command : %s\n", command);
        fprintf(stderr, "ERROR: invalid command\n");
        exit(1);
    }
}