#ifndef CSHELL_PROCESS_H
#define CSHELL_PROCESS_H

#include <sys/types.h>
#include <time.h>
#include <stdbool.h>

// Process constants
#define PROCESS_MAX_PROCESSES 100
#define PROCESS_MAX_ARGS 64
#define PROCESS_MAX_NAME 256

// Process states
typedef enum {
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_STOPPED,
    PROCESS_STATE_TERMINATED,
    PROCESS_STATE_ZOMBIE
} ProcessState;

// Process structure
typedef struct {
    pid_t pid;
    int job_id;
    char name[PROCESS_MAX_NAME];
    char **args;
    int argc;
    ProcessState state;
    int exit_code;
    bool foreground;
    time_t start_time;
    time_t end_time;
} Process;

// Process initialization and cleanup
int process_init(void);
void process_cleanup(void);

// Process operations
Process *process_create(const char *name, char **args, int argc, bool foreground);
int process_kill(Process *process, int signal);
int process_wait(Process *process);
int process_resume(Process *process);
int process_suspend(Process *process);

// Process status
ProcessState process_get_state(Process *process);
int process_get_exit_code(Process *process);

// Process retrieval
Process *process_get_by_pid(pid_t pid);
Process *process_get_by_job_id(int job_id);

// Process utilities
void process_print(Process *process);
void process_print_all(void);
void process_reap_zombies(void);

#endif // CSHELL_PROCESS_H 