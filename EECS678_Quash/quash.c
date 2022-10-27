//TO DO, add authors and stuff.

/******************************************************************************************************/
//Initialize libraries and some variables

//Libraries
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <termios.h>

//Global Variables
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
char* curr_dir;

/******************************************************************************************************/
//Basic functions
//This includes QOL and some built-in commands to Quash

//Simply print out the quash terminal
void printQuash()
{
    printf("[QUASH]$ ");
}

//Creates a process to clear out terminal
void clear_sky(){
    pid_t status = fork();
    if( status == 0 ){
        if( execvp( "clear", (char* const[]){"clear", NULL} ) == -1 ){
        printf( "error" );
        exit(0);
        }
    }
}

//Implements echo commmand
void doEcho(){
    //Prints out current path working directory
    if( argv[ 1 ] == getenv( "PWD" ) ){
        printf( "%s\n", curr_dir );
    }
    //TO DO add remaining echo functionality
}

//Implements export command
void doExport(){
    
}

//Implements pwd commmand
void doPWD(){
    if( strcmp( "pwd", argv[0] ) == 0 ){
        printf( "%s\n", curr_dir );
    }
}

//Impements the change directory command
void changeDir(){
    if( argv[1] == NULL ){
        chdir( getenv( "HOME" ) );
    } else {
        if( chdir( argv[1] ) == -1  ){
            printf( "%s: No such directory\n", argv[1]);
        }
    }
    curr_dir = getenv( "PWD" );
}

/******************************************************************************************************/

//Creates a job (another procees)
void createJob(){
    pid_t pid;
    pid = fork();
    if(pid == 0){
        setpgrp();
        if ( execvp( *argv, argv ) == -1 ){
        perror(" ");
        exit(0);
        }
    } else {
        wait(NULL);
    }
}

/******************************************************************************************************/
//Parser functions, determine what to do based on inputs

//Replaces any found $PATH found with the corresponding path directory
void findPath(){
    char* str;
    char* token;
    int len = 0;
    for( int i = 0; i < argc; i ++ ){
        str = argv[i];
        len = strlen( str );
        token = strtok( str, "$" );
        for( int j = 0; j < len; j ++ ){
            if( getenv( str + j ) ){
                str = token;
                if( getenv( str ) ){ 
                    argv[ i ] = getenv( argv[i] + j );
                } else {
                    strcat( str , getenv( argv[i] + j ) );
                    argv[ i ] = str;
                }
            }
        }
    }
}

//Parses the command given in argv
int parseCommand(){ 
    findPath();
    //Does the echo command
    /*if( strcmp( "echo", argv[0] ) == 0 ){
        doEcho();
        return( 1 );
    }*/
    //Does the pwd commands
    if( strcmp( "pwd", argv[0] ) == 0 ){
        doPWD();
        return( 1 );
    }
    //Does export commands
    if( strcmp( "export", argv[0] ) == 0 ){
        doExport();
        return( 1 );
    }
    //Change Directory command
    if( strcmp( "cd", argv[0] ) == 0 ){
        changeDir();
        return( 1 );
    }
    return( 0 );
}

//Puts the line in argV
void readLine(){
    while(argc != 0){
        argv[argc] = NULL;
        argc--;
    }
    buffer_char = 0;
    char *buff;
    while(input != '\n'){
        buffer[buffer_char++] = input;
        input = getchar();
    }
    buffer[buffer_char] = 0x00;
    buff = strtok(buffer, " ");
    while(buff != NULL){
        argv[argc] = buff;
        buff = strtok(NULL, " ");
        argc++;
    }
}

//Determines whether to exit, do built-in commands, or create a new job(process)
void doCmd(){
    //The mulitple ways of exiting quash
    if(strcmp("exit",argv[0]) ==  0 || strcmp("quit",argv[0]) ==  0){
        exit(3);
    //If it isn't a built-in command it creates a job
    } 
    if( parseCommand() == 0 ) {
        createJob();
    }
}

/******************************************************************************************************/
//Main, initializes some things and runs Quash

int main(){
    //Creates a process to empty terminal
    clear_sky();
    wait(NULL);

    //Set some variables
    parent_pid = getpid();
    parent_pgid = getpgrp();
    terminal = STDIN_FILENO;
    is_interactive = isatty(terminal);
    curr_dir = getenv( "PWD" );

    //Moved the large chunk of commented code below

    //Runs Quash
    printQuash();
    fflush(stdout);
    while(1){
        input = getchar();
        if(input == '\n'){
            printQuash();
        } else {
            readLine();
            doCmd();
            printQuash();
        }
    }
    return 0;
}

/******************************************************************************************************/
//Chunk of code
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