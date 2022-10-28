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
int background = 0;
int jobs = 0;

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
// checks for IO redirection and background or foreground execution
int checkForGroundAndRedirection()
{
    background = 0;
    int i = 0;
    while( argv[ i ] != NULL && background == 0 ){
        if( strcmp( "&", argv[i] ) == 0 ){
            background = 1;
        }
        i++;
    }
    return( 1 );
}
/******************************************************************************************************/
//Parser functions, determine what to do based on inputs

//Replaces any found $PATH found with the corresponding path directory
void findPath(){
    const int size = 1024;
    char* temp;
    char buf[ size ];
    char path[ size ];
    char result[ size ];
    int counter = 0;
    int len = 0;
    int exit;
    int replacer = 0;
    int resetter = 0;
    int pathname = 0;
    int j = 0;

    for( int i = 0; i < argc; i ++ ){
        counter = 0;
        replacer = 0;
        resetter = 0;
        pathname = 0;
        j = 0;
        len = strlen( argv[ i ] );
        for( int j = 0; j < len; j ++ ){
            if( argv[ i ][ j ] == '/' ){
                argv[ i ][ j ] = '\0';
            }
            buf[ j ] = argv[ i ][ j ];
            counter = counter + 1;
        }
        buf[ counter ] = '\0';
        buf[ counter + 1 ] = '\0';
        while( resetter != 2 ){
            result[ j + replacer ] = buf[ j + pathname ];
            if( buf[ j ] == '$' ){
                if( getenv( buf + j + 1 ) ){
                    temp = getenv( buf + j + 1 );
                    counter = 0;
                    pathname = strlen( buf + j + 1 ) + 1 + pathname;
                    for( int k = 0; k < strlen( temp ); k ++ ){
                        path[ k ] = temp[ k ];
                        counter = counter + 1;
                    }
                    path[ counter ] = '\0';
                    for( int l = 0; l < strlen( path ); l ++ ){
                        result[ j + l + replacer ] = path[ l ];
                    }
                    replacer = replacer + strlen( path ) + 1;
                    result[ j + replacer ] = '/';
                }
            }
            if( buf[ j ] == '\0' && buf[ j + 1 ] != '\0' ){
                result[ j + replacer ] = '/';
            }
            if( buf[ j ] == '\0' && buf[ j + 1 ] == '\0' ){
                resetter = 2;
            }
            j = j + 1;
        }
        strcpy( argv[ i ], result );
    }
}

//Removes all comments
int removeComments(){
    int comments = 0;
    if( argv[ 0 ][ 0 ] == '#' ){
            argv[ 0 ] = "#";
            return( 1 );
    }
    for( int i = 0; i < argc; i ++ ){
        if( argv[ i ] == "#" || argv[ i ][ 0 ] == '#' ){
            comments = 1;
        }
        if( comments == 1 ){
            argv[ i ] = "\0";
        }
        if( argv[ 0 ][ 0 ] == '#' ){
            argv[ 0 ] = "#";
        }
    }
    return ( 1 );
}

//Parses the command given in argv
int parseCommand(){ 
    findPath();
    removeComments();
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
        // Comment command so it does nothing
    if( strcmp( "#", argv[0] ) == 0 ){
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
    checkForGroundAndRedirection();
    if( parseCommand() == 0 && background == 0 ){
        execArgs();
    }
    if( background == 1 ){
        pid_t pid = fork();
        background = 0;
        jobs++;
        int currJob = jobs;
        int didCmd = 1;
        int tmpLen = argc;
        const char *tmp[15];
        char cwd[512];
        
        if( pid == 0 ){
            
            
            //print that the process started
            printf("\33[2K\r");
            printf("Background job started: [%d] %d ", currJob, getpid());
            for(int i=0; i < tmpLen; i++){
                tmp[i] = argv[i];
                printf("%s ", tmp[i]);
            }
            printf("\n");

            signal(SIGINT, SIG_IGN);

            setenv( "parent", getcwd(cwd,sizeof(cwd)), 1);
            
            // get cmd
            printQuash();
            while(didCmd){
                input = getchar();
                if(input == '\n'){
                    printQuash();
                }
                else{
                    readLine();
                    break;
                }
            }
            if( parseCommand() == 0 ){
                execArgs();
            }

            // print that process is complete
            printf("Completed: [%d] %d ", currJob, getpid());
            for(int i=0; i < tmpLen; i++){
                printf("%s ", tmp[i]);
            }
            printf("\n");
            jobs--;
        }
        else{
            wait(NULL);
            exit( 0 );
        }
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
    // parent_pid = getpid();
    // parent_pgid = getpgrp();
    // terminal = STDIN_FILENO;
    // is_interactive = isatty(terminal);

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