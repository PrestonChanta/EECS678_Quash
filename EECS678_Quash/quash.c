// TODO *Include authors, brief etc.

/**********************************************************************************/
/**********************************************************************************/
//Libraries, Definitions, and variables

//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

//Basic Definitions
#define ZERO 0 
#define TRUE 1
#define FALSE 0
#define bufMax 512
#define argMax 32

//Command line related variables
static char cmd_buf[ bufMax ];
char name_buf[ bufMax ];
static char* argv[ argMax ];
static char new_in = '\0';
static int argc = ZERO;
static int pos = ZERO;
int buf_char = ZERO;
static char** first_command;
static char** second_command;

//Quash process id's
static pid_t q_pid;
static pid_t q_pgid;
static struct termios q_tmode;
static int q_term;

//Job counter
static int active_jobs = ZERO;

/**********************************************************************************/
/**********************************************************************************/
//Classes, definitions, and more variables

struct job{
    int id;
    char* command;
    pid_t pid;
    pid_t pgid;
    int ground;
    struct job* next;
};

static struct job* curr_jobs = NULL;

/**********************************************************************************/
/**********************************************************************************/
//Basic Functions

//Simply prints out bash line
void printQuash(){
    printf("[QUASH]$ " );
}

//Cleans up terminal
void clear_sky(){
    pid_t temp = fork();
    if( temp == 0 ){
        if( execvp( "clear", ( char* const[] ){ "clear", NULL } ) == -1 ){
            printf( " " );
            exit( 0 );
        }
    }
}

//Simply takes the command line and puts them
//into the argv and argc variables
void readLine(){
    while( argc != 0 ){
        argv[ argc ] = NULL;
        argc = argc - 1;
    }
    buf_char = 0;
    char *buf;
    while( new_in != '\n' ){
        cmd_buf[ buf_char++ ] = new_in;
        new_in = getchar();
    }
    cmd_buf[ buf_char ] = 0x00;
    buf = strtok( cmd_buf, " " );
    while( buf != NULL ){
        argv[ argc ] = buf;
        buf = strtok( NULL, " " );
        argc = argc + 1;
    }
}

void execArgs(){
    if( execvp( *argv, argv ) ){
        perror( " ERR\n" );
    }
    exit( 0 );
}

/**********************************************************************************/
/**********************************************************************************/
//Built-in command functions

//
void doEcho(){
    for( int i = 1; i < argc; i ++ ){
        if( strcmp( "#", argv[ i ] ) == 0 ){
            break;
        }
        if( argv[ i ][ 0 ] == '$' ){
            argv[ i ] = "\0";
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

//Simply finds the current path, and prints it
void doPWD(){
    char cwd[ 512 ];
    getcwd( cwd, sizeof( cwd ) );
    printf( "%s\n", cwd );
}

//Finds the given path, and sets it to
//the new one.
void doExport(){
    char* temp;
    for( int i = 0; i < strlen( argv[ 1 ] ); i ++ ){
        if( argv[ 1 ][ i ] == '=' ){
            argv[ 1 ][ i ] = '\0';
            temp = argv[ 1 ] + i + 1;
            setenv( argv[ 1 ], temp, 1);
        }
    }
}

//Finds the given directory( if given )
//and uses chdir to go to it
void doCD(){
    if( argv[ 1 ] == NULL ){
        chdir( getenv( "HOME" ) );
    } else {
        if( chdir( argv[ 1 ] ) == -1  ){
            printf( "%s: No such directory\n", argv[1]);
        }
    }
}

void printJobs(){
    struct job* temp = curr_jobs;
    if( temp == NULL ){
    } else {
        while( temp != NULL ){
            printf( "[%d]    %d    %s\n", temp->id, temp->pid, temp->command);
            temp = temp->next;
        }
    }
}

/**********************************************************************************/
/**********************************************************************************/
//Parsers

//Converts all found $___ types to the according 
//path if it was found
//*NOTE doesn't work for $____$____
//*it will only convert the last one
void convertPaths(){
    const int size = 512;
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
        memset( &buf[ 0 ], 0, size );
        memset( &path[ 0 ], 0, size );
        memset( &result[ 0 ], 0, size );
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

//Converts anything found after a # symbol
//Into nothing as it is a comment, and shouldn't be read
int convertComments(){
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

int isSymbolFound( char* checker ){
    pos = 0;
    for( int i = 0; i < argc ; i ++ ){
        if( argv[ i ] == NULL ){
        } else if( strcmp( checker, argv[ i ] ) == 0 ){
            argv[ i ] = NULL;
            return( 1 );
        }
        pos = pos + 1;
    }
    return ( 0 );
}

void returnCommands( int givenPos ){
    char* buf1[ argc ];
    char* buf2[ argc ];
    memset( &buf1[ 0 ], 0, argc );
    memset( &buf2[ 0 ], 0, argc );
    int count = 0;
    int i = givenPos - 1;
    int j = givenPos + 1;
    while( argv[ i ] != NULL && argv[ i ] != "\0" &&
           strcmp( "|", argv[ i ] ) != 0  &&
           strcmp( ">", argv[ i ] ) != 0  &&
           strcmp( "<", argv[ i ] ) != 0   
                                          ){
        i = i - 1;
    }
    i = i  + 1; 
    while( argv[ i ] != NULL && argv[ i ] != "\0" &&
           strcmp( "|", argv[ i ] ) != 0  &&
           strcmp( ">", argv[ i ] ) != 0  &&
           strcmp( "<", argv[ i ] ) != 0   
                                          ){
        buf1[ count ] = argv[ i ];
        count = count + 1;
        i = i + 1;
    }
    count = 0;
    while( argv[ j ] != NULL && argv[ i ] != "\0" &&
           strcmp( "|", argv[ j ] ) != 0  &&
           strcmp( ">", argv[ j ] ) != 0  &&
           strcmp( "<", argv[ j ] ) != 0   
                                          ){
        buf2[ count ] = argv[ j ];
        count = count + 1;
        j = j + 1;
    }
    for( int i = 0; i < argc; i ++ ){
        first_command[ i ] = malloc( sizeof( buf1[ i ] ) );
        second_command[ i ] = malloc( sizeof( buf2[ i ] ) );
        strcpy( first_command[ i ], buf1[ i ] );
        strcpy( second_command[ i ], buf2[ i ] );
    }
}


//Parses through the built-in commands
//and does the according command
int parse_builtIn(){
    convertPaths();
    convertComments();
    //Quits out of quash
    if( strcmp( "exit", argv[ 0 ] ) == 0 ||
        strcmp( "quit", argv[ 0 ] ) == 0 ){
        exit( 0 );
    }
    if( strcmp( "#", argv [ 0 ] ) == 0 ){
        return( 0 );
    }
    if( strcmp( "pwd", argv[ 0 ] ) == 0 ){
        doPWD();
        return( 0 );
    }
    if( strcmp( "export", argv[ 0 ] ) == 0 ){
        doExport();
        return( 0 );
    }
    if( strcmp( "cd", argv[ 0 ] ) == 0 ){
        doCD();
        return( 0 );
    }
    if( strcmp( "jobs", argv[ 0 ] ) == 0 ){
        printJobs();
        return( 0 );
    }
    return( 1 );
}
/**********************************************************************************/
/**********************************************************************************/
//Pipes, redirects, etc.

void doPipe( int pipePos ){
    int size = argc;
    char* buf1[ size ];
    char* buf2[ size ];
    memset( &buf1[ 0 ], 0, size );
    memset( &buf2[ 0 ], 0, size );
    for( int i = 0; i < argc; i ++ ){
        if( i < pipePos - 1 ){
            buf1[ i ] = argv[ i ];
        } 
        if( i > pipePos ){
            buf2[ i ] = argv[ i ];
        }
    }
    int fds[ 2 ];
    pipe( fds );
    int pid;
    pid = fork();
    if( pid == 0 ){
        dup2( fds[ 1 ], STDIN_FILENO );
        close( fds[ 0 ] ); close( fds[ 1 ] );
        if( execvp( buf1[ 1 ], buf1 ) ){
            perror( " " );
        }
        exit( 0 );
    } else {
        dup2( fds[ 0 ], STDIN_FILENO );
        close( fds[ 0 ] ); close( fds[ 1 ] );
        if( execvp( buf2[ 1 ], buf2 ) ){
            perror( " " );
        }
    }
}

void doRedirectIn( int redirectPos ){
    char* file_name = argv[ redirectPos + 1 ];
    int fdin = open( file_name, O_RDONLY );
    dup2( fdin, STDIN_FILENO );
    close( fdin );
}

void doRedirectOut( int redirectPos ){
    char* file_name = argv[ redirectPos + 1 ];
    int fdout = open( file_name, O_CREAT | O_WRONLY | O_TRUNC, 0666 );
    dup2( fdout, STDIN_FILENO );
    if( parse_builtIn() ){
        execArgs();
    } 
    close( fdout );
}


/**********************************************************************************/
/**********************************************************************************/
//Job related functions

//Simply creates the new job and sets its variables
//It then goes to add it to curr_jobs listclear
struct job* addJob( pid_t pid, char* name, int job_type){
    struct job* newjob = malloc( sizeof( struct job ) );
    newjob->command = ( char* ) malloc( sizeof( name ) );
    newjob->command = strcpy( newjob->command, name );
    newjob->ground = job_type;
    newjob->pid = pid;
    newjob->next = NULL;
    if( curr_jobs == NULL ){
        active_jobs = active_jobs + 1;
        newjob->id = active_jobs;
        return( newjob );
    } else {
        struct job* temp = curr_jobs;
        while( temp->next != NULL ){
            temp = temp->next;
        }
        newjob->id = temp->id + 1;
        temp->next = newjob;
        active_jobs = active_jobs + 1;
        return( curr_jobs );
    }
}

//Deletes a job from the curr_jobs list
struct job* delJob( struct job* given ){
    if( curr_jobs == NULL ){
        return( NULL );
    }
    struct job* curr;
    struct job* prev;
    curr = curr_jobs->next;
    prev = curr_jobs;
    if( prev->pid == given->pid ){
        prev = NULL;
        active_jobs = active_jobs - 1;
        return prev;
    }
    while( curr != NULL ){
        if( curr->pid == given->pid ){
            active_jobs = active_jobs - 1;
            prev->next = curr->next;
        }
        prev = curr;
        curr = curr->next;
    }
    return curr_jobs;
}

//Goes through the curr_jobs list and
//finds whether the given process ID exists
struct job* getJob( int given ){
    struct job* finder = curr_jobs;
    while( finder != NULL ){
        if( finder->pid == given ){
            return( finder );
        } else {
            finder = finder->next;
        }
    }
    return( NULL );
}

void jobBack( struct job* given ){
    given->ground = 0;
    tcsetpgrp( q_term, q_pgid );
}

void jobHandler(){
    pid_t pid;
    int status;
    pid = waitpid( WAIT_ANY, &status, WNOHANG | WUNTRACED );
    if ( pid > 0 ){
        struct job* temp = getJob( pid );
        if( temp == NULL ){
            return;
        }
        if( WIFEXITED( status ) ){
            if( temp->ground == 1 ){
                printf( "\nCompleted: [%d]    %d    %s\n", temp->id, temp->pid, temp->command );
            }
            curr_jobs = delJob( temp );
        }
        tcsetpgrp( q_term, q_pgid );
    }
}

void createChildProc( int given ){
    char* temp = NULL;
    pid_t pid;
    if( given == 0 ){
        pid = fork();
        if( pid == 0 ){
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGCHLD, &jobHandler);
            signal(SIGTTIN, SIG_DFL);
            setpgrp();
            if( isSymbolFound( "|" ) ){
                returnCommands( pos );
                doPipe( pos );
            }
            if( isSymbolFound( "<" ) ){
                doRedirectIn( pos );
            }
            if( isSymbolFound( ">" ) ){
                doRedirectOut( pos );
            }
            execArgs();
            exit( 0 );  
        } else {
            wait( NULL );
        }
    } else {
        pid = fork();
        if( pid == 0 ){
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            signal(SIGCHLD, &jobHandler);

            signal(SIGTTIN, SIG_DFL);
            setpgrp();
            if( isSymbolFound( "|" ) ){
                doPipe( pos );
            }
            if( isSymbolFound( "<" ) ){
                doRedirectIn( pos );
            }
            if( isSymbolFound( ">" ) ){
                doRedirectOut( pos );
            }
            execArgs();
            exit( 0 );  
        } else {
            setpgid(pid, pid );
            curr_jobs = addJob( pid, *argv, 1 );
            printf( "Background job started: [%d]    %d    %s\n", active_jobs, pid, *argv );
        }
    }
}

void doJob(){
    int job_type = 0;
    job_type = isSymbolFound( "&" );

    createChildProc( job_type );
    usleep(1000);
}

/**********************************************************************************/
/**********************************************************************************/
//Initializer & Packagers( nesting multiple functions )

//
void initializeQuash(){
    q_pid = getpid();
    q_pgid = getpgrp();
    q_term = STDIN_FILENO;

    if( isatty( q_term ) ){
        while( tcgetpgrp( q_term ) != getpgrp() ){
            kill( q_pid, SIGTTIN );
            }
        signal( SIGQUIT, SIG_IGN );
        signal( SIGTTOU, SIG_IGN );
        signal( SIGTTIN, SIG_IGN );
        signal( SIGTSTP, SIG_IGN );
        signal( SIGINT, SIG_IGN );
        signal(SIGCHLD, &jobHandler );
        setpgid( q_pid, q_pid );
        q_pgid = getpgrp();
        if( tcsetpgrp( q_term, q_pgid ) == -1 ){
            tcgetattr( q_term, &q_tmode );
        }
    } else {
        printf( "Quash was unable to initialize.\n Exiting...\n" );
        exit( 0 );
    }
    printQuash();
    fflush( stdout );
}

//Simply parses through built-ins
//and if it isn't, it is presumed to be a job
void doCmd(){
        if( parse_builtIn() ){
            doJob( argv );
        }
}

/**********************************************************************************/
/**********************************************************************************/
//Main

//
int main(){
    clear_sky();
    wait( NULL );
    printf( "Welcome...\n" );
    initializeQuash();
    while( TRUE ){
        new_in = getchar();
        if( new_in == '\n' ){
            printQuash();

        } else {
            readLine();
            doCmd();
            printQuash();
        }
    }
}