#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>
#include <termios.h>

pid_t parent_pid;
pid_t parent_pgid;
int terminal = STDIN_FILENO;
int is_interactive;
static struct termios parent_tmodes;
char input; 
static char* myargv[15];
static int myargc = 0;

void printBash()
{
    printf("[BASH]$ ");
}

int main()
{
    parent_pid = getpid();
    parent_pgid = getpgrp();
    terminal = STDIN_FILENO;
    is_interactive = isatty(terminal);

    if(is_interactive)
    {
        while(tcgetpgrp(terminal) != (parent_pgid))
            kill(parent_pid,SIGTTIN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        //signal(SIGCHLD, &signalHandler_child);
        setpgid(parent_pid,parent_pid);
        parent_pgid = getpgrp();
        
        if(tcsetpgrp(terminal,parent_pgid) == -1)
            tcgetattr(terminal,&parent_tmodes);
        char* curr_dir = (char*)calloc(1024,sizeof(char));
    }
    printBash();
    fflush(stdout);
    while(1)
    {
        input = getchar();
        if(input == '\n')
            printBash();
        else
        {
            printBash();
        }
    }

    return 0;
}