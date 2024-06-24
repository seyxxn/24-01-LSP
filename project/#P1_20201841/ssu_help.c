#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void helpBackup()
{
    printf("  > backup <PATH> [OPTION]... : backup file if <PATH> is file\n");
    printf("    -d : backup files in directory if <PATH> is directory\n");
    printf("    -r : backup files in directory recursive if <PATH> is directory\n");
    printf("    -y : backup file although already backuped\n");
}

void helpRemove()
{
    printf("  > remove <PATH> [OPTION]... : remove backuped file if <PATH> is file\n");
    printf("    -d : remove backuped files in directory if <PATH> is directory\n");
    printf("    -r : remove backuped files in directory recursive if <PATH> is directory\n");
    printf("    -a : remove all backuped files\n");
}

void helpRecover()
{
    printf("  > recover <PATH> [OPTION]... : recover backuped file if <PATH> is file\n");
    printf("    -d : recover backuped files in directory if <PATH> is directory\n");
    printf("    -r : recover backuped files in directory recursive if <PATH> is directory\n");
    printf("    -l : recover latest backuped file\n");
    printf("    -n <NEW_PATH> : recover backuped file with new path\n");
}

void helpList()
{
    printf("  > list [PATH] : show backup list by directory structure\n");
    printf("    >> rm <INDEX> [OPTION]... : remove backuped files of [INDEX] with [OPTION]\n");
    printf("    >> rc <INDEX> [OPTION]... : recover backuped files of [INDEX] with [OPTION]\n");
    printf("    >> vi(m) <INDEX> : edit original file of [INDEX]\n");
    printf("    >> exit : exit program\n");
}

void helpHelp(){
    printf("  > help [COMMAND] : show commands for program\n");
}

void help(char *command)
{
    if (!strcmp(command, "all"))
    {
        printf("Usage:\n");
        helpBackup();
        helpRemove();
        helpRecover();
        helpList();
        helpHelp();
    }
    else if (!strcmp(command, "backup"))
    {
        printf("Usage:\n");
        helpBackup();
    }
    else if (!strcmp(command, "remove"))
    {
         printf("Usage:\n");
        helpRemove();
    }
    else if (!strcmp(command, "recover"))
    {
         printf("Usage:\n");
         helpRecover();
    }
    else if (!strcmp(command, "list"))
    {
         printf("Usage:\n");
         helpList();
    }
    else if (!strcmp(command, "help"))
    {
         printf("Usage:\n");
         helpHelp();
    }
    else
    {
        fprintf(stderr, "ERROR: invalid command\n");
        exit(1);
    }

    printf("\n\n");
}