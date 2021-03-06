#ifndef __FUNC_H__
#define __FUNC_H__

#include <sys/resource.h>
#include <sys/types.h>

#define MAX_STR_LENGTH 1000
#define MAX_ARR_LENGTH 1000
#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

// Global variables
char *ENV_VARS[MAX_ARR_LENGTH];

/* a struct holding output information from stats command
** -u   the cpu time spent in user mode
** -s   the cpu time spent in system/kernel mode
** -p   the hard page faults
** -v   the voluntary context switches
** -i    the involuntary context switches
*/
struct Stats {
	int u,s,p,v,i;
} STATS;

// Functions
int split_str_by_char(char *buf, char **argv, char token);
void substitute_env_vars_no_space(char *str, char * result);
int count_pipes(char **argv);
int good_piping_format(char **argv);
void print_if_error(int status, char * mess);

int stats_enabled(struct Stats st);
void print_enabled_stats(struct Stats st);
void enable_stats(char **argv, struct Stats *st);
void display_stats(struct Stats st);

void set_env_var(char **argv);

int run_pipe_commands(char **expanded_argv, int pipes, pid_t *pid);

void cowsay( char **argv );
#endif