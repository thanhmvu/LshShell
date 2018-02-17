#include "signal_handler.h"

/* sigchld_handler - when a child process has stopped or terminated */
void sigchld_handler( int sig ) {
	int old_errno = errno;
	int status;
	pid_t pid;

	sigset_t mask_all, prev_all;
	Sigfillset(&mask_all);

	while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0) {
		// if child process terminates, either normally or by signal
		if ( WIFEXITED(status) || WIFSIGNALED(status) ) {
			// if process in foreground
			if ( pid == get_foreground_pid() ) {
				set_foreground_pid(0);
				// when user types Ctrl + C or /bin/kill/, display a terminate success message
				if ( WIFSIGNALED(status) ) {
					printf("Job %d terminated by signal: Interrupt\n", pid);
				}
			}

			// delete the terminated job from job array
			Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
			delete_job(pid);
			Sigprocmask(SIG_SETMASK, &prev_all, NULL);
		}

		// if job stopped by Ctrl + Z
		if ( WIFSTOPPED(status) ) {
			// if process in foreground
			if (pid == get_foreground_pid()) {
				set_foreground_pid(0);
			}
			// update job status to STOPPED
			Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
			set_job_status( get_job_from_pid(pid), STOPPED);
			Sigprocmask(SIG_SETMASK, &prev_all, NULL);

			// print success message
			printf("Job [%d] %d stopped by signal: Stopped\n", get_jid_from_pid(pid), pid);
		}
	}

	errno = old_errno;
}

/* sigint_handler - when user enters ctrl + C, 
** if there's a foreground job, terminate it
** else terminate the shell
*/
void sigint_handler( int sig ) {
	// if there is a job running in the foreground, terminate it
	if ( get_foreground_pid() != 0 ) {
		Kill( get_foreground_pid(), SIGINT );
	}
	// if no job running in the foreground, terminate the shell
	else {
		// revert action to SIGINT to default - which is to terminate this process
		Signal( SIGINT, SIG_DFL );
		Kill( getpid(), SIGINT );   	// getpid returns the pid of the calling process
	}
}

/* sigstop_handler - when user enters ctrl + Z, 
** if there's a foreground job, stop it
** else stop the shell
*/
void sigstop_handler( int sig ) {
	// if there is a job running in the foreground, stop it
	if ( get_foreground_pid() != 0 ) {
		Kill( get_foreground_pid(), SIGTSTP );
	}
	// if no job running in the foreground, stop the shell
	else {
		// revert action to SIGSTP to default - which is to stop this process
		Signal( SIGTSTP, SIG_DFL );
		Kill( getpid(), SIGTSTP );    	// getpid returns the pid of the calling process
	}
}