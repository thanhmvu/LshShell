#include "csapp.h"
#define MAXARGS 128

/* Function prototypes */
void eval( char *cmdline );
int parseline( char *buf, char **argv );
int builtin_command( char **argv ); 

int main() {
	char cmdline[MAXLINE];

	while (1) {
		// print special prompt signaling that we are in the shell
		printf( "lsh> " );

		// get the command line from stdin
		Fgets(cmdline, MAXLINE, stdin);

		// exit on error
		if ( feof(stdin) ) 
			exit(0);

		// evaluate and execute input command
		eval( cmdline );
	}
	return 0;
}

/* eval - Evaluate a command line */
void eval( char *cmdline ) {
	char *argv[MAXARGS];	// argument list for execve() 
	char buf[MAXLINE];		// holds modified command line
	int bg;					// indicate whether the job is in background or foreground
	pid_t pid;				// process id

	// copy the command line as a string to buf so that it can be modified and parsed
	strcpy( buf, cmdline );

	// parse the command line and set value for bg
	bg = parseline( buf, argv );

	// ignore empty lines
	if (argv[0] == NULL) return;

	if ( !builtin_command(argv) ) {
		// create a child running user's job
		if ( (pid = Fork()) == 0 ) {
			if ( execve(argv[0], argv, environ) < 0 ) {
				printf( "%s: Command not found.\n", argv[0] );
				exit(0);
			}
		}

		// parent waits for background job to terminate
		if (!bg) {
			int status;
			if ( waitpid(pid, &status, 0) < 0 ) 
				unix_error( "waifg: waitpid error" );
		} else {
			printf( "%d %s", pid, cmdline );
		}
	}
}

/* If first arg is a built-in command, run it and return true */
int builtin_command( char **argv ) {
	// if arg[0] == "quit", execute it imemdiately and exit
	if ( strcmp(argv[0], "quit") == 0 ) exit(0);
	
	// ignore singleton &
	if ( strcmp(argv[0], "&") == 0 ) return 1;

	// not a built in command
	return 0;
}

/* parseline - Parse the command line and build the argv array */
int parseline( char *buf, char **argv ) {
	char *delim;		// points to first space delimiter
	int argc;			// number of argss
	int bg;				// background job?

	// replace trailing '\n' with space
	buf[strlen(buf) - 1] = ' ';

	// ignore leading space
	while ( *buf && (*buf == ' ') )
		buf ++;

	// build the argv list
	argc = 0;
	while ( (delim = strchr(buf, ' ')) ) {	// strchr: locate 1st occurence of character string
		argv[argc ++] = buf;
		// null-char at the end of every string
		*delim = '\0';
		buf = delim + 1;
		// ignore space
		while ( *buf && (*buf == ' ') ) 
			buf ++;
	}

	// null terminator for argv array
	argv[argc] = NULL;

	// ignore blank lines
	if (argc == 0) return 1;

	// should the job run in the background?
	bg = *argv[argc - 1] == '&';
	if ( bg != 0 ) 
		argv[--argc] = NULL;
	
	return bg;
}

