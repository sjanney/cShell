#include "../../include/shell/commands.h"
#include "../../include/shell/shell.h"
#include "../../include/shell/process.h"
#include "../../include/shell/env.h"
#include "../../include/shell/ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>

// Color definitions
#define COLOR_RESET     "\033[0m"
#define COLOR_BOLD      "\033[1m"
#define COLOR_RED       "\033[31m"
#define COLOR_GREEN     "\033[32m"
#define COLOR_YELLOW    "\033[33m"
#define COLOR_BLUE      "\033[34m"
#define COLOR_MAGENTA   "\033[35m"
#define COLOR_CYAN      "\033[36m"

// Function declarations
int cmd_help(int argc, char **argv);
int cmd_exit(int argc, char **argv);
int cmd_clear(int argc, char **argv);
int cmd_ls(int argc, char **argv);
int cmd_cd(int argc, char **argv);
int cmd_pwd(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_rmdir(int argc, char **argv);
int cmd_touch(int argc, char **argv);
int cmd_rm(int argc, char **argv);
int cmd_cat(int argc, char **argv);
int cmd_echo(int argc, char **argv);
int cmd_ps(int argc, char **argv);
int cmd_kill(int argc, char **argv);
int cmd_bg(int argc, char **argv);
int cmd_fg(int argc, char **argv);
int cmd_jobs(int argc, char **argv);
int cmd_env(int argc, char **argv);
int cmd_export(int argc, char **argv);
int cmd_unset(int argc, char **argv);
int cmd_ai_help(int argc, char **argv);
int cmd_ai_explain(int argc, char **argv);
int cmd_ai_suggest(int argc, char **argv);
int cmd_ai_learn(int argc, char **argv);
int cmd_sysmon(int argc, char **argv);

// Command table
Command builtin_commands[] = {
    { "help", "Display help information", cmd_help },
    { "exit", "Exit the shell", cmd_exit },
    { "clear", "Clear the screen", cmd_clear },
    { "ls", "List directory contents", cmd_ls },
    { "cd", "Change directory", cmd_cd },
    { "pwd", "Print working directory", cmd_pwd },
    { "mkdir", "Create a new directory", cmd_mkdir },
    { "rmdir", "Remove an empty directory", cmd_rmdir },
    { "touch", "Create an empty file", cmd_touch },
    { "rm", "Remove a file", cmd_rm },
    { "cat", "Display file contents", cmd_cat },
    { "echo", "Display a line of text", cmd_echo },
    { "ps", "List processes", cmd_ps },
    { "kill", "Terminate a process", cmd_kill },
    { "bg", "Resume a stopped job in background", cmd_bg },
    { "fg", "Resume a stopped job in foreground", cmd_fg },
    { "jobs", "List background jobs", cmd_jobs },
    { "env", "Display environment variables", cmd_env },
    { "export", "Set an environment variable", cmd_export },
    { "unset", "Remove an environment variable", cmd_unset },
    { "ai", "AI assistant commands", cmd_ai_help },
    { "ai-help", "Show AI command help", cmd_ai_help },
    { "ai-explain", "Explain a command", cmd_ai_explain },
    { "ai-suggest", "Get command suggestions", cmd_ai_suggest },
    { "ai-learn", "Provide feedback to AI", cmd_ai_learn },
    { "sysmon", "Display system metrics (CPU, Memory, Disk, Load)", cmd_sysmon },
    { NULL, NULL, NULL }
};

// Help command
int cmd_help(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    
    printf(COLOR_CYAN COLOR_BOLD "Built-in Commands:\n" COLOR_RESET);
    printf("  " COLOR_GREEN "help" COLOR_RESET "     - Show this help message\n");
    printf("  " COLOR_GREEN "exit" COLOR_RESET "     - Exit the shell\n");
    printf("  " COLOR_GREEN "clear" COLOR_RESET "    - Clear the screen\n");
    printf("  " COLOR_GREEN "ls" COLOR_RESET "       - List files in a directory\n");
    printf("  " COLOR_GREEN "cd" COLOR_RESET "       - Change directory\n");
    printf("  " COLOR_GREEN "pwd" COLOR_RESET "      - Print working directory\n");
    printf("  " COLOR_GREEN "mkdir" COLOR_RESET "    - Create a directory\n");
    printf("  " COLOR_GREEN "rmdir" COLOR_RESET "    - Remove a directory\n");
    printf("  " COLOR_GREEN "touch" COLOR_RESET "    - Create a file\n");
    printf("  " COLOR_GREEN "rm" COLOR_RESET "       - Remove a file\n");
    printf("  " COLOR_GREEN "cat" COLOR_RESET "      - Display file contents\n");
    printf("  " COLOR_GREEN "echo" COLOR_RESET "     - Display a message\n");
    printf("  " COLOR_GREEN "ps" COLOR_RESET "       - List processes\n");
    printf("  " COLOR_GREEN "kill" COLOR_RESET "     - Kill a process\n");
    printf("  " COLOR_GREEN "bg" COLOR_RESET "       - Resume a process in the background\n");
    printf("  " COLOR_GREEN "fg" COLOR_RESET "       - Resume a process in the foreground\n");
    printf("  " COLOR_GREEN "jobs" COLOR_RESET "     - List background jobs\n");
    printf("  " COLOR_GREEN "env" COLOR_RESET "      - Display environment variables\n");
    printf("  " COLOR_GREEN "export" COLOR_RESET "   - Set an environment variable\n");
    printf("  " COLOR_GREEN "unset" COLOR_RESET "    - Unset an environment variable\n");
    return 0;
}

// Exit command
int cmd_exit(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    
    exit(0);
    return 0;
}

// Clear command
int cmd_clear(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    
    printf("\033[2J\033[H");
    return 0;
}

// List directory contents
int cmd_ls(int argc, char **argv) {
    char *dir = ".";
    if (argc > 1) {
        dir = argv[1];
    }
    
    DIR *d = opendir(dir);
    if (!d) {
        printf(COLOR_RED "ls: cannot access '%s': %s\n" COLOR_RESET, dir, strerror(errno));
        return 1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        
        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                printf(COLOR_BLUE "%s/\n" COLOR_RESET, entry->d_name);
            } else if (st.st_mode & S_IXUSR) {
                printf(COLOR_GREEN "%s*\n" COLOR_RESET, entry->d_name);
            } else {
                printf("%s\n", entry->d_name);
            }
        }
    }
    
    closedir(d);
    return 0;
}

// Change directory
int cmd_cd(int argc, char **argv) {
    char *dir = getenv("HOME");
    if (argc > 1) {
        dir = argv[1];
    }
    
    if (chdir(dir) != 0) {
        printf(COLOR_RED "cd: %s: %s\n" COLOR_RESET, dir, strerror(errno));
        return 1;
    }
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        setenv("PWD", cwd, 1);
    }
    
    return 0;
}

// Print working directory
int cmd_pwd(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("getcwd");
        return 1;
    }
}

// Create directory
int cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "mkdir: missing operand\n" COLOR_RESET);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (mkdir(argv[i], 0755) != 0) {
            printf(COLOR_RED "mkdir: cannot create directory '%s': %s\n" COLOR_RESET, 
                   argv[i], strerror(errno));
            return 1;
        }
    }
    return 0;
}

// Remove directory
int cmd_rmdir(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "rmdir: missing operand\n" COLOR_RESET);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (rmdir(argv[i]) != 0) {
            printf(COLOR_RED "rmdir: failed to remove '%s': %s\n" COLOR_RESET, 
                   argv[i], strerror(errno));
            return 1;
        }
    }
    return 0;
}

// Create empty file
int cmd_touch(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "touch: missing operand\n" COLOR_RESET);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_CREAT | O_WRONLY, 0644);
        if (fd < 0) {
            printf(COLOR_RED "touch: cannot touch '%s': %s\n" COLOR_RESET, 
                   argv[i], strerror(errno));
            return 1;
        }
        close(fd);
    }
    return 0;
}

// Remove file
int cmd_rm(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "rm: missing operand\n" COLOR_RESET);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (unlink(argv[i]) != 0) {
            printf(COLOR_RED "rm: cannot remove '%s': %s\n" COLOR_RESET, 
                   argv[i], strerror(errno));
            return 1;
        }
    }
    return 0;
}

// Display file contents
int cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "cat: missing operand\n" COLOR_RESET);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            printf(COLOR_RED "cat: %s: %s\n" COLOR_RESET, argv[i], strerror(errno));
            continue;
        }
        
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
            fwrite(buf, 1, n, stdout);
        }
        
        fclose(f);
    }
    return 0;
}

// Echo command
int cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], (i < argc - 1) ? " " : "");
    }
    printf("\n");
    return 0;
}

// List processes
int cmd_ps(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    process_print_all();
    return 0;
}

// Kill process
int cmd_kill(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "kill: missing operand\n" COLOR_RESET);
        return 1;
    }
    
    int signal = SIGTERM;
    int start_arg = 1;
    
    if (argv[1][0] == '-') {
        signal = atoi(argv[1] + 1);
        start_arg = 2;
    }
    
    for (int i = start_arg; i < argc; i++) {
        pid_t pid = atoi(argv[i]);
        Process *process = process_get_by_pid(pid);
        
        if (!process) {
            printf(COLOR_RED "kill: process %d not found\n" COLOR_RESET, pid);
            continue;
        }
        
        if (process_kill(process, signal) != 0) {
            printf(COLOR_RED "kill: failed to kill process %d: %s\n" COLOR_RESET, 
                   pid, strerror(errno));
        }
    }
    return 0;
}

// Background process
int cmd_bg(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "bg: missing job ID\n" COLOR_RESET);
        return 1;
    }
    
    int job_id = atoi(argv[1]);
    Process *process = process_get_by_job_id(job_id);
    
    if (!process) {
        printf(COLOR_RED "bg: job %d not found\n" COLOR_RESET, job_id);
        return 1;
    }
    
    if (process_resume(process) != 0) {
        printf(COLOR_RED "bg: failed to resume job %d\n" COLOR_RESET, job_id);
        return 1;
    }
    
    return 0;
}

// Resume job in foreground
int cmd_fg(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "fg: missing job ID\n" COLOR_RESET);
        return 1;
    }
    
    // Get job ID
    int job_id = atoi(argv[1]);
    if (job_id <= 0) {
        printf(COLOR_RED "fg: invalid job ID\n" COLOR_RESET);
        return 1;
    }
    
    // Find process
    Process *process = process_get_by_job_id(job_id);
    if (!process) {
        printf(COLOR_RED "fg: no such job\n" COLOR_RESET);
        return 1;
    }
    
    // Resume process
    if (process_resume(process) != 0) {
        printf(COLOR_RED "fg: failed to resume job\n" COLOR_RESET);
        return 1;
    }
    
    // Wait for process to complete
    int status = process_wait(process);
    if (status < 0) {
        printf(COLOR_RED "fg: failed to wait for job\n" COLOR_RESET);
        return 1;
    }
    
    return status;
}

// List jobs
int cmd_jobs(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    process_print_all();
    return 0;
}

// List environment variables
int cmd_env(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    int count;
    char **env_list = env_get_all(&count);
    if (!env_list) {
        printf(COLOR_RED "Error: Failed to get environment variables\n" COLOR_RESET);
        return 1;
    }

    for (int i = 0; i < count; i++) {
        printf("%s\n", env_list[i]);
    }

    // Free the environment list
    for (int i = 0; i < count; i++) {
        free(env_list[i]);
    }
    free(env_list);

    return 0;
}

// Export environment variable
int cmd_export(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "export: missing variable name\n" COLOR_RESET);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        char *eq = strchr(argv[i], '=');
        if (!eq) {
            printf(COLOR_RED "export: invalid syntax: %s\n" COLOR_RESET, argv[i]);
            continue;
        }
        
        *eq = '\0';
        if (env_set(argv[i], eq + 1) != 0) {
            printf(COLOR_RED "export: failed to set %s\n" COLOR_RESET, argv[i]);
        }
    }
    return 0;
}

// Unset environment variable
int cmd_unset(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "unset: missing variable name\n" COLOR_RESET);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (env_unset(argv[i]) != 0) {
            printf(COLOR_RED "unset: failed to unset %s\n" COLOR_RESET, argv[i]);
        }
    }
    return 0;
}

// AI commands
int cmd_ai_help(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    printf(COLOR_CYAN "\nAI Assistant Commands:\n" COLOR_RESET);
    printf("  ai explain <command>  - Explain what a command does\n");
    printf("  ai suggest <task>     - Get command suggestions\n");
    printf("  ai learn <input> <feedback> - Provide feedback for learning\n");
    printf("  ai help              - Show this help message\n\n");
    return 0;
}

int cmd_ai_explain(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "Error: Please provide a command to explain\n" COLOR_RESET);
        return 1;
    }

    char *explanation = ai_explain_command(argv[1]);
    if (explanation) {
        printf(COLOR_CYAN "\nExplanation:\n" COLOR_RESET);
        printf("%s\n\n", explanation);
        free(explanation);
        return 0;
    }
    return 1;
}

int cmd_ai_suggest(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_RED "Error: Please describe what you want to do\n" COLOR_RESET);
        return 1;
    }

    char *suggestion = ai_suggest_command(argv[1]);
    if (suggestion) {
        printf(COLOR_CYAN "\nSuggested command:\n" COLOR_RESET);
        printf("%s\n\n", suggestion);
        free(suggestion);
        return 0;
    }
    return 1;
}

int cmd_ai_learn(int argc, char **argv) {
    if (argc < 3) {
        printf(COLOR_RED "Error: Please provide input and feedback\n" COLOR_RESET);
        return 1;
    }

    ai_learn(argv[1], argv[2]);
    return 0;
}

// System monitoring command
int cmd_sysmon(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    // Get CPU usage using sysctl
    int mib[2] = {CTL_HW, HW_NCPU};
    int num_cpu;
    size_t len = sizeof(num_cpu);
    if (sysctl(mib, 2, &num_cpu, &len, NULL, 0) == 0) {
        printf("CPU Cores: %d\n", num_cpu);
    }

    // Get memory usage using mach
    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t host_size = sizeof(vm_statistics64_data_t) / sizeof(integer_t);
    vm_size_t page_size;
    vm_statistics64_data_t vm_stats;
    
    host_page_size(host_port, &page_size);
    
    if (host_statistics64(host_port, HOST_VM_INFO64, (host_info64_t)&vm_stats, &host_size) == KERN_SUCCESS) {
        unsigned long long total_mem = 0;
        int mib[2] = {CTL_HW, HW_MEMSIZE};
        size_t len = sizeof(total_mem);
        if (sysctl(mib, 2, &total_mem, &len, NULL, 0) == 0) {
            unsigned long long free_mem = vm_stats.free_count * page_size;
            unsigned long long used_mem = total_mem - free_mem;
            float mem_usage = (float)used_mem / total_mem * 100.0f;
            
            printf("\nMemory Usage: %.1f%%\n", mem_usage);
            printf("Total Memory: %.2f GB\n", total_mem / 1024.0f / 1024.0f / 1024.0f);
            printf("Used Memory: %.2f GB\n", used_mem / 1024.0f / 1024.0f / 1024.0f);
            printf("Free Memory: %.2f GB\n", free_mem / 1024.0f / 1024.0f / 1024.0f);
        }
    }

    // Get disk usage
    FILE *df = popen("df -h /", "r");
    if (df) {
        char line[256];
        // Skip header
        fgets(line, sizeof(line), df);
        if (fgets(line, sizeof(line), df)) {
            char filesystem[256], size[32], used[32], avail[32], use_percent[32], mounted[256];
            sscanf(line, "%s %s %s %s %s %s",
                   filesystem, size, used, avail, use_percent, mounted);
            printf("\nDisk Usage:\n");
            printf("Filesystem: %s\n", filesystem);
            printf("Size: %s\n", size);
            printf("Used: %s\n", used);
            printf("Available: %s\n", avail);
            printf("Use%%: %s\n", use_percent);
            printf("Mounted on: %s\n", mounted);
        }
        pclose(df);
    }

    // Get load average using sysctl
    struct loadavg load;
    int mib_load[2] = {CTL_VM, VM_LOADAVG};
    size_t load_size = sizeof(load);
    if (sysctl(mib_load, 2, &load, &load_size, NULL, 0) == 0) {
        printf("\nLoad Average (1/5/15 min): %.2f %.2f %.2f\n",
               (double)load.ldavg[0] / load.fscale,
               (double)load.ldavg[1] / load.fscale,
               (double)load.ldavg[2] / load.fscale);
    }

    // Get process count
    FILE *ps = popen("ps aux | wc -l", "r");
    if (ps) {
        char line[32];
        if (fgets(line, sizeof(line), ps)) {
            int process_count = atoi(line) - 1; // Subtract header line
            printf("Total Processes: %d\n", process_count);
        }
        pclose(ps);
    }

    return 0;
} 