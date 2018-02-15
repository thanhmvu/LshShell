#include "csapp.h"
#include "signal_handler.h"
#include "job.h"

/* sigchld_handler - when a child process has stopped or terminated */
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
		Kill( getpid(), SIGINT );		// getpid returns the pid of the calling process
	}
}

/* sigstop_handler - when user enters ctrl + Z, 
** if there's a foreground job, stop it
** else stop the shell
 */
void sigstop_handler( int sig ) {
	// if there is a job running in the foreground, stop it
	if ( get_foreground_pid() != 0 ) {
		fprintf(stderr, "puccaaaaa %d %d \n", get_foreground_pid(), getpid() );
		Kill( get_foreground_pid(), SIGTSTP );
	}
	// if no job running in the foreground, stop the shell
	else {
		// revert action to SIGSTP to default - which is to stop this process
		Signal( SIGTSTP, SIG_DFL );
		Kill( getpid(), SIGTSTP );		// getpid returns the pid of the calling process
	}
}