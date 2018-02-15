#include "signal_handler.h"
#define MAXARGS 128

/* Function prototypes */
void eval( char *cmdline );
int parseline( char *buf, char **argv );
int builtin_command( char **argv );

int main(int argc, char **argv) {

	if (Signal(SIGCHLD, sigchild_handler) == SIG_ERR)
	  unix_error("signal child handler error");
	if (Signal(SIGINT, sigint_handler) == SIG_ERR)
	  unix_error("signal int handler error");
	if (Signal(SIGTSTP, sigstop_handler) == SIG_ERR)
	  unix_error("signal stop handler error");

	char cmdline[MAXLINE];		// the string holding the command line
	init_jobs();				// initialize the array of jobs

	while (1) {
		// print special prompt signaling that we are in the shell
		printf( "lsh> " );

		// get the command line from stdin
		Fgets(cmdline, MAXLINE, stdin);

		// exit when reaching end of file
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

	// if build in command, execute it immediately
	if ( builtin_command(argv) ) return;
	
	sigset_t mask, prev_mask;
	// Set up mask to indicate SIGCHLD should be blocked
	Sigemptyset( &mask );
	Sigaddset( &mask, SIGCHLD );

	// Disable SIGCHLD Signal
	Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

	// create a child running user's job
	if ( (pid = Fork()) == 0 ) {
		// unblock signal in child process
		Sigprocmask( SIG_SETMASK, &prev_mask, NULL );

		// create process group
		Setpgid(0, 0);

		if ( execvp(argv[0], argv) < 0 ) {
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
      set_foreground_pid( pid );
      fprintf(stderr, "foregroup pid %d \n", pid);
      while(get_foreground_pid())
        sigsuspend(&prev_mask);
    }
    else
      printf("[%d] %d %s \t %s\n", new_jid, pid, "Running", cmdline);

	// unblock child signal
	Sigprocmask( SIG_SETMASK, &prev_mask, NULL );
	
}

/* If first arg is a built-in command, run it and return true */
int builtin_command( char **argv ) {
	// if arg[0] == "quit", execute it imemdiately and exit
	if ( strcmp(argv[0], "quit") == 0 ) exit(0);
	
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
			printf("invalid format.");
		}

		return 1;
	}

	if ( strcmp(argv[0], "bg") == 0 ) {
		// get the job's pid or jid
		int id = parse_id( argv[1] );

		if ( id != -1 && argv[2] == NULL ) {
			bring_job_to_background( id, argv[1] );
		} else {
			printf("invalid format.");
		}

		return 1;
	}
	
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

void sigchild_handler(int sig) {
  int old_errno = errno;
  int status;
  pid_t pid;

  sigset_t mask_all, prev_all;
  Sigfillset(&mask_all);

  /* exit or be stopped or continue */
  while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {
    /* exit normally */
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      if ( pid == get_foreground_pid() ) {
        set_foreground_pid(0);
      } else {
        Sio_puts("pid "); Sio_putl(pid); Sio_puts(" terminates\n");
      }
      Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      delete_job(pid);
      Sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    /* be stopped */
    if (WIFSTOPPED(status)) {
    	fprintf(stderr, "stoppedddd");
      if (pid == get_foreground_pid()) {
        set_foreground_pid(0);
      }
      // set pid status stopped
      Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      Job *jp = get_job_from_pid(pid);
      if ( jp ) jp->status = STOPPED;
      Sigprocmask(SIG_SETMASK, &prev_all, NULL);

      Sio_puts("pid "); Sio_putl(pid); Sio_puts(" be stopped\n");
    }

    /* continue */
    if(WIFCONTINUED(status)) {
      set_foreground_pid(pid);
      // set pid status running
      Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      Job *jp = get_job_from_pid(pid);
      if ( jp ) jp->status = RUNNING;
      Sigprocmask(SIG_SETMASK, &prev_all, NULL);
      Sio_puts("pid "); Sio_putl(pid); Sio_puts(" continue\n");
    }
  }

  errno = old_errno;
}