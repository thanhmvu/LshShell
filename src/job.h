#include "csapp.h"
#define MAXJOBS 128

// ***** typedef **** //
typedef enum job_status { RUNNING, STOPPED, TERMINATED } job_status;
typedef struct {
	int pid;				// process ID
	int jid;				// job ID
	job_status status;		// job status
	int in_use;				// whether job is still in use (not deleted)
	char command[MAXLINE];	// the name of the command that runs this job
} Job;

// ***** variables **** //
static Job jobs[MAXJOBS];						// the array of current jobs (those not deleted)
static volatile sig_atomic_t foreground_pid;	// the pid of the job currently running in foreground

// ***** functions **** //
void init_jobs();
void list_jobs();
int create_job(pid_t, char *);
void deleteJob(pid_t);