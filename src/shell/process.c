#include "../../include/shell/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

// Global process table
static Process process_table[PROCESS_MAX_PROCESSES];
static int process_count = 0;
static int next_job_id = 1;

// Signal handler for child processes
static void sigchld_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    // Check all processes for terminated children
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].pid > 0) {
            int status;
            pid_t pid = waitpid(process_table[i].pid, &status, WNOHANG);
            
            if (pid == process_table[i].pid) {
                // Process has terminated, update its status
                if (WIFEXITED(status)) {
                    process_table[i].exit_code = WEXITSTATUS(status);
                    process_table[i].state = PROCESS_STATE_TERMINATED;
                } else if (WIFSIGNALED(status)) {
                    process_table[i].exit_code = WTERMSIG(status);
                    process_table[i].state = PROCESS_STATE_TERMINATED;
                } else if (WIFSTOPPED(status)) {
                    process_table[i].state = PROCESS_STATE_STOPPED;
                }
                
                // Set end time
                process_table[i].end_time = time(NULL);
            }
        }
    }
}

// Initialize process subsystem
int process_init(void) {
    // Clear process table
    memset(process_table, 0, sizeof(process_table));
    process_count = 0;
    next_job_id = 1;
    
    // Set up signal handler for child processes
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
    
    return 0;
}

// Clean up process subsystem
void process_cleanup(void) {
    // Kill any remaining processes
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].state == PROCESS_STATE_RUNNING ||
            process_table[i].state == PROCESS_STATE_STOPPED) {
            kill(process_table[i].pid, SIGTERM);
        }
        // Free arguments
        if (process_table[i].args) {
            for (int j = 0; j < process_table[i].argc; j++) {
                free(process_table[i].args[j]);
            }
            free(process_table[i].args);
        }
    }
    
    process_count = 0;
}

// Create a new process
Process *process_create(const char *name, char **args, int argc, bool foreground) {
    if (!name || !args || argc <= 0 || argc >= PROCESS_MAX_ARGS) {
        return NULL;
    }
    
    // Check if we have space for a new process
    if (process_count >= PROCESS_MAX_PROCESSES) {
        return NULL;
    }
    
    // Allocate a new process entry
    Process *process = &process_table[process_count];
    memset(process, 0, sizeof(Process));
    
    // Copy name
    strncpy(process->name, name, PROCESS_MAX_NAME - 1);
    
    // Allocate and copy arguments
    process->args = (char **)malloc(argc * sizeof(char *));
    if (!process->args) {
        return NULL;
    }
    for (int i = 0; i < argc; i++) {
        process->args[i] = strdup(args[i]);
    }
    process->argc = argc;
    
    // Initialize other fields
    process->state = PROCESS_STATE_RUNNING;
    process->exit_code = 0;
    process->job_id = next_job_id++;
    process->foreground = foreground;
    process->start_time = time(NULL);
    process->end_time = 0;
    
    // Fork the process
    pid_t pid = fork();
    if (pid < 0) {
        // Error forking
        for (int i = 0; i < argc; i++) {
            free(process->args[i]);
        }
        free(process->args);
        return NULL;
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args);
        // If exec fails
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        process->pid = pid;
        process_count++;
        
        // Wait for foreground process
        if (foreground) {
            process->state = PROCESS_STATE_RUNNING;
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                process->exit_code = WEXITSTATUS(status);
                process->state = PROCESS_STATE_TERMINATED;
            } else if (WIFSIGNALED(status)) {
                process->exit_code = WTERMSIG(status);
                process->state = PROCESS_STATE_TERMINATED;
            } else if (WIFSTOPPED(status)) {
                process->state = PROCESS_STATE_STOPPED;
            }
            
            process->end_time = time(NULL);
        }
        
        return process;
    }
}

// Kill a process
int process_kill(Process *process, int signal) {
    if (!process || process->state != PROCESS_STATE_RUNNING) {
        return -1;
    }
    
    return kill(process->pid, signal);
}

// Wait for a process to terminate
int process_wait(Process *process) {
    if (!process) {
        return -1;
    }
    
    int status;
    pid_t pid = waitpid(process->pid, &status, 0);
    
    if (pid == process->pid) {
        if (WIFEXITED(status)) {
            process->exit_code = WEXITSTATUS(status);
            process->state = PROCESS_STATE_TERMINATED;
        } else if (WIFSIGNALED(status)) {
            process->exit_code = WTERMSIG(status);
            process->state = PROCESS_STATE_TERMINATED;
        } else if (WIFSTOPPED(status)) {
            process->state = PROCESS_STATE_STOPPED;
        }
        
        process->end_time = time(NULL);
        return process->exit_code;
    }
    
    return -1;
}

// Resume a stopped process
int process_resume(Process *process) {
    if (!process || process->state != PROCESS_STATE_STOPPED) {
        return -1;
    }
    
    if (kill(process->pid, SIGCONT) == 0) {
        process->state = PROCESS_STATE_RUNNING;
        return 0;
    }
    
    return -1;
}

// Suspend a running process
int process_suspend(Process *process) {
    if (!process || process->state != PROCESS_STATE_RUNNING) {
        return -1;
    }
    
    if (kill(process->pid, SIGSTOP) == 0) {
        process->state = PROCESS_STATE_STOPPED;
        return 0;
    }
    
    return -1;
}

// Get process state
ProcessState process_get_state(Process *process) {
    if (!process) {
        return PROCESS_STATE_TERMINATED;
    }
    
    return process->state;
}

// Get process exit code
int process_get_exit_code(Process *process) {
    if (!process) {
        return -1;
    }
    
    return process->exit_code;
}

// Find process by PID
Process *process_get_by_pid(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].pid == pid) {
            return &process_table[i];
        }
    }
    
    return NULL;
}

// Find process by job ID
Process *process_get_by_job_id(int job_id) {
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].job_id == job_id) {
            return &process_table[i];
        }
    }
    
    return NULL;
}

// Print process information
void process_print(Process *process) {
    if (!process) {
        return;
    }
    
    char state_char = '?';
    switch (process->state) {
        case PROCESS_STATE_RUNNING:    state_char = 'R'; break;
        case PROCESS_STATE_STOPPED:    state_char = 'S'; break;
        case PROCESS_STATE_TERMINATED: state_char = 'T'; break;
        case PROCESS_STATE_ZOMBIE:     state_char = 'Z'; break;
    }
    
    printf("[%d] %5d %c %s\n", 
           process->job_id, 
           process->pid, 
           state_char, 
           process->name);
}

// Print all processes
void process_print_all(void) {
    printf("JOB   PID  S COMMAND\n");
    for (int i = 0; i < process_count; i++) {
        process_print(&process_table[i]);
    }
}

// Reap zombie processes
void process_reap_zombies(void) {
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].state == PROCESS_STATE_TERMINATED) {
            // Free arguments
            if (process_table[i].args) {
                for (int j = 0; j < process_table[i].argc; j++) {
                    free(process_table[i].args[j]);
                }
                free(process_table[i].args);
            }
            
            // Move last process to this slot
            if (i < process_count - 1) {
                process_table[i] = process_table[process_count - 1];
                i--; // Check this slot again
            }
            
            process_count--;
        }
    }
} 