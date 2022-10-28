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

//TO DO: The echo command has a lot of functionality based on parsing
//it needs to 
//1: print strings/path variables
//2: remove single or double quotes (on the ends of each arg)
//3: recognize comments with #

//Implements echo commmand
void doEcho(){
    //TO DO add remaining echo functionality
    for( int i = 1; i < argc; i ++ ){
        if( strcmp( "#", argv[ i ] ) == 0 ){
            break;
        }
        if( strcmp( "\"", argv[ i ] ) == 0 || 
            strcmp( "\"", argv[ i ] ) == 0 ){
            argv[ i ] = "\0";
        }
        if( argv[ i ][ 0 ] == '\'' || 
            argv[ i ][ 0 ] == '"' ){
            argv[ i ] = argv[ i ] + 1;
        }
        if( argv[ i ][ strlen( argv[ i ] ) - 1 ] == '\'' || 
            argv[ i ][ strlen( argv[ i ] ) - 1 ] == '"' ){
            argv[ i ][ strlen( argv[ i ] ) - 1 ] = '\0';
        }
        if( strlen( argv[ i ] ) != 0 ){
            printf( "%s ", argv[ i ] );
        }
    }
    printf("\n");
}

//Implements export command
void doExport(){
    char* temp;
    temp = argv[1];
    for( int i = 0; i < strlen( argv[ 1 ] ); i ++ ){
        if( argv[ 1 ][ i ] == '=' ){
            temp[ i ] = '\0';
            setenv( temp, argv[ 1 ] + i + 1, 1);
        }
    }
}

//Implements pwd commmand
void doPWD(){
    char cwd[512];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd );
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
}

/******************************************************************************************************/

//Creates a job (another procees)
void execArgs(){
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
    char* temp;
    int len = 0;
    for( int i = 0; i < argc; i ++ ){
        temp = argv[ i ];
        len = strlen( argv[ i ] );
        for( int j = 0; j < len; j ++ ){
            if( argv[ i ][ j ] == '$' && getenv( argv[ i ] + j + 1) ){
                temp[ j ] = '\0';
                strcat( temp, getenv( argv[ i ] + j + 1) );
                argv[ i ] = temp;
            }
        }
    }
}

//Parses the command given in argv
int parseCommand(){ 
    findPath();
    //TO DO: Fix the echo command so this can be uncommented
    //Does the echo command 
    if( strcmp( "echo", argv[0] ) == 0 ){
        doEcho();
        return( 1 );
    }
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
    //How to exit quashs
    if(strcmp("exit",argv[0]) ==  0 || strcmp("quit",argv[0]) ==  0){
        exit(3);
    //If it isn't a built-in command it creates a job
    } 
    if( parseCommand() == 0 ) {
        execArgs();
    } 
}

/******************************************************************************************************/
//Main, initializes some things and runs Quash

int main(){
    //Creates a process to empty terminal
    clear_sky();
    wait(NULL);
    printf("Welcome...\n");

    //Set some variables
    parent_pid = getpid();
    parent_pgid = getpgrp();
    terminal = STDIN_FILENO;
    is_interactive = isatty(terminal);

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