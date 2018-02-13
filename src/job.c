#include "job.h"

void init_jobs() {
	memset(jobs, 0, sizeof(jobs));
}

/* list_job - list all jobs running in the background */
void list_jobs() {
	for ( int i = 0 ; i < MAXJOBS ; i++ ) {
		Job j = jobs[i];
		if (j.in_use) {
			char *status = (j.status == RUNNING ? "Running" : "Stopped");
			printf( "[%d] %d %s \t %s\n", j.jid, j.pid, status, j.command );
		}
	}
}

/* find_next_usable_jid - find the smallest jid of a job that has been deleted
** the returned jid will be assigned to a new job
*/
int find_next_usable_jid() {
	int jid = -1;
	for ( int i = 0 ; i < MAXJOBS ; i++ ) {
		if ( !jobs[i].in_use ) {
			jid = i;
			break;
		}
	}
	return jid;
}

/* create_job - create a new Job and return its job ID */
int create_job(pid_t pid, char *cmdline) {
	int jid;
	if ( (jid = find_next_usable_jid()) == -1 ) 
		unix_error("Error: Maximum number of jobs reached. Cannot create new job.\n");
	Job new_job = jobs[jid];
	new_job.jid = jid;
	new_job.pid = pid;
	new_job.status = RUNNING;
	strcpy( new_job.command, cmdline );
	new_job.in_use = 1;
	return jid;
}

/* delete_job - delete the Job whose pid matches the input pid */
void delete_job(pid_t pid) {
	for ( int i = 0 ; i < MAXJOBS ; i++ ) {
		if ( jobs[i].pid == pid && jobs[i].in_use ) {
			jobs[i].in_use = 0;
		}
	}
}