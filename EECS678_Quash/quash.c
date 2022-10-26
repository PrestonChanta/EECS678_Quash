#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <termios.h>

pid_t parent_pid;
pid_t parent_pgid;
int terminal = STDIN_FILENO;
int is_interactive;
static struct termios parent_tmodes;
char input; 
static char* argv[15];
static int argc = 0;
int buffer_char = 0;
char buffer[100];

void printBash()
{
    printf("[BASH]$ ");
}

// puts the line in argv var
void readLine()
{
    while(argc != 0)
    {
        argv[argc] = NULL;
        argc--;
    }
    buffer_char = 0;
    char *buff;
    while(input != '\n')
    {
        buffer[buffer_char++] = input;
        input = getchar();
    }
    buffer[buffer_char] = 0x00;
    buff = strtok(buffer, " ");
    while(buff != NULL)
    {
        argv[argc] = buff;
        buff = strtok(NULL, " ");
        argc++;
    }
}

void doCmd()
{
    if(strcmp("exit",argv[0]) ==  0 || strcmp("quit",argv[0]) ==  0)
        exit(3);
    else
    {
        pid_t pid;
        pid = fork();
        if(pid == 0)
        {
            setpgrp();
            // does the command (echo $HOME and echo $PWD don't work)
            if(execvp(*argv, argv) == -1)
                perror(" ");
                exit(0);
        }
        else
        {
            wait(NULL);
        }
    }
}

int main()
{
    parent_pid = getpid();
    parent_pgid = getpgrp();
    terminal = STDIN_FILENO;
    is_interactive = isatty(terminal);
    
    printf("Welcome...\n");
/*
    if(is_interactive)
    {
        while(tcgetpgrp(terminal) != (parent_pgid))
            kill(parent_pid,SIGTTIN);

        signal(SIGQUIT, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGCHLD, &signalHandler_child);

        setpgid(parent_pid,parent_pid);
        parent_pgid = getpgrp();
        
        if(tcsetpgrp(terminal,parent_pgid) == -1)
            tcgetattr(terminal,&parent_tmodes);
        char* curr_dir = (char*)calloc(1024,sizeof(char));
    }
*/
    printBash();
    fflush(stdout);
    while(1)
    {
        input = getchar();
        if(input == '\n')
            printBash();
        else
        {
            readLine();
            doCmd();
            printBash();
        }
    }

    return 0;
}