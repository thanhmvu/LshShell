#include "utilities.h"
#include "csapp.h"

/*
 * Create pipes and run commands
 */
int run_pipe_commands(char **expanded_argv, int pipes, pid_t *pid) {
	if (!good_piping_format(expanded_argv)) {
		fprintf(stderr, "[ERROR] Bad piping format. Required: command1 | command2 | ... \n");
	} else {
		int i, pipefd_write, pipefd_read;
		int processes = pipes + 1;
		
		/* Create all pipes */
		int pipefd[pipes][2];
		for (i=0; i < pipes; i++) {
			if (pipe(pipefd[i]) == -1) {
				fprintf(stderr, "[ERROR] Failed to create pipe at i = %d\n", i);
			}
		}
		
		/* Loop to run pipe commands */
		char **child_argv;
		char **next_argv = expanded_argv;
		for (i=0; i < processes; i++) {
			child_argv = next_argv;

			/* Move pointer to next argument list */
			while(*next_argv != NULL && strcmp(*next_argv,"|")) {
				next_argv++;
			}
			*next_argv = NULL;
			next_argv++;

			/* Create child process */
			*pid = Fork();
			if (*pid == -1) {
				fprintf(stderr, "[ERROR] Fork: Failed to fork process at i = %d\n", i);
			} 
			
			/* Child runs user job */
			else if (*pid == 0) { 
				
				/* Dup read end */
				if (i != 0) {
					pipefd_read = pipefd[i-1][PIPE_READ_END];
					if (dup2(pipefd_read, STDIN_FILENO) == -1) {
						fprintf(stderr, "[C] [ERROR] Error while dup read end: %s\n", strerror(errno));
					}
				} 
				
				/* Dup write end */
				if (i != processes-1) {
					pipefd_write = pipefd[i][PIPE_WRITE_END];
					if (dup2(pipefd_write, STDOUT_FILENO) == -1) {
						fprintf(stderr, "[C] [ERROR] Error while dup write end: %s\n", strerror(errno));
					}
				} 
				
				/* Child closes all pipes after dup */
				for (i=0; i < pipes; i++) {
					close(pipefd[i][0]);
					close(pipefd[i][1]);
				}
				
				/* Execute command */
				execvp(child_argv[0], child_argv);
			}
		}
		
		/* Parent closes all pipes after children have done dup */
		for (i=0; i < pipes; i++) {
			close(pipefd[i][0]);
			close(pipefd[i][1]);
		}
	}
	
	return 0;
}


void set_env_var(char **argv) {
	
	char *delim = strchr(argv[0], '=');
	if (delim) {
		
		/* Split into key, val */
		char *key = argv[0];
		*delim = '\0';
		char *val = delim + 1;

		/* Unset variable */
		if (!strcmp(val, "")) {
			unsetenv(key);
		} 
		
		/* Set variable */
		else {
			setenv(key, val, 1); /* overwrite if key exists */
		}
	}
}

void run_stats(char **argv, struct Stats *st) {
	int i,j;
	for(i=1; argv[i] != NULL; i++) {
		
		if (argv[i][0] != '-') {
			fprintf(stderr, "Invalid option: %s. Missing '-'\n", argv[i]);
		} 
		
		else {
			for(j=1; argv[i][j] != '\0'; j++) {
				char ch = argv[i][j];
				switch (ch) {
					
					case 'c' :
						 *st = (struct Stats) {.u=0, .s=0,  .p=0,  .v=0,  .i=0};
						break;
					
					case 'a' :
						 *st = (struct Stats) {.u=1, .s=1,  .p=1,  .v=1,  .i=1};
						break;
					
					case 'l' :
						print_enabled_stats(*st);
						break;
					
					case 'u' :
						(*st).u = 1;
						break;
					
					case 's' :
						(*st).s = 1;
						break;
					
					case 'p' :
						(*st).p = 1;
						break;
					
					case 'v' :
						(*st).v = 1;
						break;
					
					case 'i' :
						(*st).i = 1;
						break;						
					
					default :
						fprintf(stderr, "Invalid option: %c\n", ch);
				}
			}
		}
	}
	return;
}

int stats_enabled(struct Stats st) {
	if (st.u || st.s || st.p || st.v || st.i) {
		return 1;
	}
	return 0;
}

void display_stats(struct Stats st) {
	struct rusage u;
	
	if (getrusage(RUSAGE_CHILDREN, &u) == -1) {
		fprintf(stderr, "Error when getrusage\n");
	} 
	
	else {
		if (st.u)
			fprintf(stderr,"user mode: %ld.%06ld seconds\n", u.ru_utime.tv_sec, u.ru_utime.tv_usec);
		
		if (st.s)
			fprintf(stderr,"sys mode: %ld.%06ld seconds\n", u.ru_stime.tv_sec, u.ru_stime.tv_usec);
		
		if (st.p)
			fprintf(stderr,"hard page faults: %ld\n", u.ru_majflt);
		
		if (st.v)
			fprintf(stderr,"voluntary context switches: %ld\n", u.ru_nvcsw);
		
		if (st.i)
			fprintf(stderr,"involuntary context switches: %ld\n", u.ru_nivcsw);
	}
}

void print_enabled_stats(struct Stats st) {
	
	if (stats_enabled(st)) {
		if (st.u)
			fprintf(stderr,"u: user mode is enabled\n");
		
		if (st.s)
			fprintf(stderr,"s: sys mode is enabled\n");
		
		if (st.p)
			fprintf(stderr,"p: hard page faults is enabled\n");
		
		if (st.v)
			fprintf(stderr,"v: voluntary context switches is enabled\n");
		
		if (st.i)
			fprintf(stderr,"i: involuntary context switches is enabled\n");	
	} 
	
	else {
		fprintf(stderr,"No stats enabled\n");
	}
}

void get_pipe_argv(char **argv, char ***commands) {
	commands[0] = argv;
	int i = 1;
	
	for(; *argv != NULL; argv++) {
		if (!strcmp(*argv,"|")) {
			*argv = NULL;
			commands[i++] = argv + 1;
		}
	}
}

void print_if_error(int status, char * mess) {
	if (status == -1) {
		fprintf(stderr, "[ERROR] %s\n", mess);
	}
}

int count_pipes(char **argv) {
	int cnt = 0;
	int i;
	
	for(i=0; argv[i] != NULL; i++) {
		if (!strcmp(argv[i],"|"))
			cnt ++;
	}
	
	return cnt;
}

int good_piping_format(char **argv) {
	/* Check if first arg is NULL or a pipe */
	if (argv[0] == NULL || !strcmp(argv[0],"|")) {
		return 0;
	}
	
	/* Check for two consecutive pipes */
	int i;
	for(i=1; argv[i] != NULL; i++) {
		if (!strcmp(argv[i],"|") && !strcmp(argv[i-1],"|")) {
			return 0;
		}
	}
	
	/* Check if last arg is a pipe */
	if (!strcmp(argv[i-1],"|")) {
		return 0;
	}
	
	return 1;
}

int split_str_by_char(char *_str, char **argv, char token) {
	char *delim; 
	int argc = 0; 

	char *buf = malloc(strlen(_str) + 1);
	strcpy(buf,_str);
	
	argv[argc++] = buf;
	while ((delim = strchr(buf, token))) {
		*delim = '\0';
		buf = delim + 1;
		argv[argc++] = buf;
	}
	argv[argc] = NULL;

	return argc;
}

void substitute_env_vars_no_space(char *str, char * result) {
	/*
	missing behaviors:
		$$ -> 23165
		$$$ -> 23165$
	*/
	char token = '$';
	int i, size;

	char *words[MAX_STR_LENGTH];
	split_str_by_char(str, words, token);
	
	size = strlen(words[0]);
	strncpy(result, words[0], size);
	int cnt = size;
	
	for (i=1; words[i] != NULL; i++) {
		char *val = getenv(words[i]);
		
		if (val) {
			size = strlen(val);
			strncpy(result + cnt, val, size);
			cnt += size;
		} 
	}
	result[cnt]='\0';
}


int split_env_var(char _var_val[], char *delim, char *results[2]) {
  char var_val[MAX_STR_LENGTH];
  strcpy(var_val,_var_val);
  results[0] = strtok_r(var_val, delim, &results[1]);
	
	return 0;
}

int split_str(char str0[], char *delim, char *results[]) {
  char str[MAX_STR_LENGTH];
  strcpy(str,str0);
  
  int length;
  char *token = strtok(str, delim);
  
	for (length=0; token != NULL; length++)
  {
    results[length] = token;
    token = strtok(NULL, delim);
  }
	
  return length;
}