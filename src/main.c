#include "signal_handler.h"
#include "utilities.h"

#define MAXARGS 128

/* Function prototypes */
void eval( char *cmdline );
int parse_line( char *buf, char **argv );
int builtin_command( char **argv );

int main(int argc, char **argv) {

	// register signal handler for when child finishes
	if (Signal(SIGCHLD, sigchld_handler) == SIG_ERR)
		unix_error("signal child handler error");

	// register Ctrl + C signal handler
	if (Signal(SIGINT, sigint_handler) == SIG_ERR)
		unix_error("signal int handler error");

	// register Ctrl + Z signal handler
	if (Signal(SIGTSTP, sigstop_handler) == SIG_ERR)
		unix_error("signal stop handler error");

	char cmdline[MAXLINE];		// the string holding the command line
	init_jobs();				// initialize the array of jobs

	STATS = (struct Stats) {0, 0, 0, 0, 0};

	while (1) {
		// print special prompt signaling that we are in the shell
		char *lshprompt = getenv("lshprompt");
		printf("%s> ", (lshprompt) ? lshprompt : "lsh"); 

		// get the command line from stdin
		Fgets(cmdline, MAXLINE, stdin);

		// exit when reaching end of file
		if ( feof(stdin) ) {
			exit(0);
		}

		// evaluate and execute input command
		eval( cmdline );
	}
	return 0;
}

void eval( char *cmdline ) {
	char *argv[MAXARGS];  	// argument list for execve() 
	char buf[MAXLINE];    	// holds modified command line
	int bg;         		// indicate whether the job is in background or foreground
	pid_t pid;        		// process id

	// copy the command line as a string to buf so that it can be modified and parsed
	strcpy( buf, cmdline );

	// parse the command line and set value for bg
	bg = parse_line( buf, argv );

	// ignore empty lines
	if (argv[0] == NULL) return;

	// if built-in command, execute it immediately
	if ( builtin_command(argv) ) return;


	// Substitue environment variables
	char *expanded_argv[MAX_ARR_LENGTH];
	int i, j = 0;
	for(i = 0; argv[i] != NULL ; i++) {

		// Look up and replace environment variable
		char *str = malloc(MAX_STR_LENGTH);
		substitute_env_vars_no_space(argv[i], str);
		
		// Add non-empty output to new argv array
		if(strcmp(str,"")) {
			expanded_argv[j] = str;
			j++;
		}
	}
	expanded_argv[j] = NULL; 	// null-terminated array
	int cnt_expanded_argv = i;	// save array length

	// Set up mask to indicate SIGCHLD should be blocked
	sigset_t mask, prev_mask;
	Sigemptyset( &mask );
	Sigaddset( &mask, SIGCHLD );

	// Disable SIGCHLD Signal
	Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

	// piping
	int pipes = count_pipes(expanded_argv);
	if (pipes > 0) {
		run_pipe_commands(expanded_argv, pipes, &pid);
	}
	
	// create a child running user's job
	else if ( (pid = Fork()) == 0 ) {
		// unblock signal in child process
		Sigprocmask( SIG_SETMASK, &prev_mask, NULL );

		// create process group
		Setpgid(0, 0);

		// execute bash command with environment variables
		if ( execvp(expanded_argv[0], expanded_argv) < 0 ) {
			printf( "%s: Command not found.\n", argv[0] );
			exit(0);
		}
	}

	// blocking signal to protect the code below
	sigset_t mask_all, prev_all;
	Sigfillset( &mask_all );
	Sigprocmask( SIG_BLOCK, &mask_all, &prev_all );

	// save the job information to job array
	int new_jid = create_job(pid, cmdline);

	// unblock signal when saving is done
	Sigprocmask( SIG_SETMASK, &prev_all, NULL );

	if ( !bg ) {
		// update the foreground pid tracker	
		set_foreground_pid( pid );

		// while a process is running in foreground, block the REPL
		while(get_foreground_pid()) {
			sigsuspend(&prev_mask);
		}
	}
	else
		printf("[%d] %d %s \t %s\n", new_jid, pid, "Running", cmdline);

	// enable SIGCHLD signal again
	Sigprocmask( SIG_SETMASK, &prev_mask, NULL );

	// free memory 
	for ( int i = 0 ; i < cnt_expanded_argv ; i++ ) {
		free(expanded_argv[i]);
	}
}

/*
 * If first arg is a builtin command, run it and return true;
 * else return false.
 */
int builtin_command( char **argv ) {
	// if arg[0] == "quit", execute it imemdiately and exit
	if ( strcmp(argv[0], "quit") == 0 || !strcmp(argv[0], "q")) exit(0);

	// ignore singleton &
	if ( strcmp(argv[0], "&") == 0 ) return 1;

	// lists all background jobs
	if ( strcmp(argv[0], "jobs") == 0 ) {
		list_jobs();
		return 1;
	}

	// fb <job id>
	if ( strcmp(argv[0], "fg") == 0 ) {
		// get the job's pid or jid
		int id = parse_id( argv[1] );

		if ( id != -1 && argv[2] == NULL ) {
			bring_job_to_foreground( id, argv[1] );
		} else {
			printf("Invalid format.");
		}

		return 1;
	}

	// bg <job id>
	if ( strcmp(argv[0], "bg") == 0 ) {
		// get the job's pid or jid
		int id = parse_id( argv[1] );

		if ( id != -1 && argv[2] == NULL ) {
			bring_job_to_background( id, argv[1] );
		} else {
			printf("Invalid format.");
		}

		return 1;
	}
	
	/* Set environment variables */
	if (strchr(argv[0], '=')) {
		if (argv[1] != NULL) { /* too many arguments */
			printf("%s: Command not found.\n", argv[1]);
		} else {
			set_env_var(argv);
		}
		return 1;
	}
	
	/* Enable stats */
	if (!strcmp(argv[0], "stats")) {
		run_stats(argv, &STATS);
		return 1;
	}

	// not a built in command
	return 0;
}

/* parseline - Parse the command line and build the argv array */
int parse_line( char *buf, char **argv ) {
	char *delim;    // points to first space delimiter
	int argc;     	// number of argss
	int bg;       	// background job?

	// replace trailing '\n' with space
	buf[strlen(buf) - 1] = ' ';

	// ignore leading space
	while ( *buf && (*buf == ' ') )
	buf ++;

	// build the argv list
	argc = 0;
	while ( (delim = strchr(buf, ' ')) ) {  // strchr: locate 1st occurence of character string
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

/* parse_id - get the input id from an argv string of form "%id" or "id" */
int parse_id(char *argv) {
	if ( argv == NULL ) return -1;

	int id;

	// format fg/bg %id
	if ( *argv == '%' && strlen(argv) >= 2 ) {
		id = atoi(argv + 1);
		// if atoi returns 0 and argv is not "%0", argv is in invalid format
		if ( id == 0 && strcmp(argv, "%0") != 0) {
		  return -1;
		}
	}
	// format fg/bg id
	else {
		id = atoi(argv);
		// if atoi returns 0 and argv is not "0", argv is in invalid format
		if ( id == 0 && strcmp(argv, "0") != 0 ) 
		  return -1;
	}

	return id;
}

