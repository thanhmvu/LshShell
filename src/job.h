#include "csapp.h"
#define MAXJOBS 128

// ***** typedef **** //
typedef enum job_status { RUNNING, STOPPED } job_status;
typedef struct {
	pid_t pid;				// process ID
	job_status status;		// job status
	int in_use;				// whether job is still in use (not deleted)
	char command[MAXLINE];	// the name of the command that runs this job
} Job;

// ***** variables **** //
static Job jobs[MAXJOBS];						// the array of current jobs (those not deleted)
static volatile sig_atomic_t foreground_pid;	// the pid of the job currently running in foreground

// ***** functions **** //

// modifying job array
void init_jobs();
void list_jobs();
int create_job(pid_t, char *);
void delete_job(pid_t);

// getters and setters for foreground id
void set_foreground_pid(sig_atomic_t);
pid_t get_foreground_pid();

// modifying job environment (fore / back ground)
void bring_job_to_foreground(int, char *);
void bring_job_to_background(int, char *);

// get a Job pointer from its id
Job *get_job_from_pid(pid_t pid);
Job *get_job_from_jid(int jid);
int get_jid_from_pid(pid_t pid);

void set_job_status(Job *job, job_status status);