#include "job.h"

void init_jobs() {
    memset(jobs, 0, sizeof(jobs));
}

/* list_job - list all jobs running */
void list_jobs() {
    for ( int i = 0 ; i < MAXJOBS ; i++ ) {
        Job j = jobs[i];
        if (j.in_use) {
            char *status = (j.status == RUNNING ? "Running" : "Stopped");
            printf( "[%d] %d %s \t %s\n", i, j.pid, status, j.command );
        }
    }
}

/* find_next_usable_jid - find the smallest jid of a job that has been deleted
** the returned jid will be assigned to a new job
*/
int find_next_usable_jid() {
    for ( int i = 0 ; i < MAXJOBS ; i++ ) {
        if ( !jobs[i].in_use ) return i;
    }
    return -1;
}

/* create_job - create a new Job and return its job ID */
int create_job(pid_t pid, char *cmdline) {
    int jid;
    // find a usable jid
    if ( (jid = find_next_usable_jid()) == -1 ) 
        unix_error("Error: Maximum number of jobs reached. Cannot create new job.\n");

    // save information to job
    jobs[jid].pid = pid;
    jobs[jid].status = RUNNING;
    strcpy( jobs[jid].command, cmdline );
    jobs[jid].in_use = 1;

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

// ***** getters and setters **** //

/* get_job_from_pid - get a job in the jobs array whose pid matches input id */
Job *get_job_from_pid( pid_t pid ) {
    for ( int i = 0 ; i < MAXJOBS ; i++ ) {
        if ( jobs[i].pid == pid )
            return &jobs[i];
    }
    return NULL;
}

/* get_job_from_jid - get a job in the jobs array whose jid (array index) matches input id */
Job *get_job_from_jid( int jid ) {
    if (jid >= 0 && jid < MAXJOBS )
        return &(jobs[jid]);
    return NULL;
}

/* bring_job_to_foreground - bring the job with the input id to the foreground */
void bring_job_to_foreground( int id, char *input ) {
    sigset_t mask, prev_mask;

    // signal blocking to protect the code below
    Sigemptyset( &mask );
    Sigaddset( &mask, SIGCHLD );
    Sigprocmask( SIG_BLOCK, &mask, &prev_mask );

    // get the job pid, either based on the parsed id or by looking up the job array
    pid_t pid = ( input[0] == '%' ? get_job_from_jid(id)->pid : id );

    // send SIGCONT to job
    Kill( pid, SIGCONT );

    // run job it foreground
    set_foreground_pid( pid );

    // block the REPL until this foreground job ends
    while ( get_foreground_pid(pid) ) {
        sigsuspend( &prev_mask );
    }

    // unblock child signal
    Sigprocmask( SIG_SETMASK, &prev_mask, NULL );
}

/* bring_job_to_foreground - bring the job with the input id to the foreground */
void bring_job_to_background( int id, char *input ) {
    // get the job pid, either based on the parsed id or by looking up the job array
    pid_t pid = ( input[0] == '%' ? get_job_from_jid(id)->pid : id );

    // send SIGCONT to job
    Kill( pid, SIGCONT );
}

/* set the foreground_pid variable to the id of the job currently running on foreground */
void set_foreground_pid( pid_t id)  {
    foreground_pid = id;
}

/* get the pid of the job running in foreground */
pid_t get_foreground_pid() {
    return foreground_pid;
}

/* set the job status */
void set_job_status( Job *job, job_status status ) {
    if ( job ) 
        job->status = status;
}