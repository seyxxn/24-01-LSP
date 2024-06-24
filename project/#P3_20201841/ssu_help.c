#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void helpAdd()
{
    printf("add <PATH> [OPTION]... : add new daemon process of <PATH> if <PATH> is file\n");
    printf("      -d : add new daemon process of <PATH> if <PATH> is directory\n");
    printf("      -r : add new daemon process of <PATH> recursive if <PATH> is directory\n");
    printf("      -t <TIME> : set daemon process time to <TIME> sec (default : 1sec)\n");
}

void helpRemove()
{
    printf("remove <DAEMON_PID> : delete daemon process with <DAEMON_PID>\n");
}

void helpList()
{
    printf("list [DAEMON_PID] : show daemon process list or dir tree\n");
}

void helpHelp(){
    printf("help [COMMAND] : show commands for program\n");
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
        printf("    > "); helpList();
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
    else if (!strcmp(command, "list"))
    {
        printf("Usage: "); helpList(); printf("\n");
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