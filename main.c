#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LINE 80
#define MAX_ARGS 10
#define MAX_HISTORY 1000
#define MAX_JOBS 100
#define MAX_FILES 1000
#define MAX_DIRS 100
#define MAX_USERS 10
#define MAX_PROCESSES 100
#define MAX_MEMORY_BLOCKS 100
#define MAX_NETWORK_INTERFACES 4
#define MAX_DEVICES 10
#define HISTORY_FILE ".shell_history"
#define VFS_ROOT "/vfs"

// Virtual File System structures
typedef struct {
    char name[MAX_LINE];
    char content[MAX_LINE * 10];
    int size;
    char owner[MAX_LINE];
    char permissions[10];
    time_t created;
    time_t modified;
} VFile;

typedef struct {
    char name[MAX_LINE];
    VFile *files[MAX_FILES];
    int file_count;
    char owner[MAX_LINE];
    char permissions[10];
    time_t created;
} VDirectory;

// User structure
typedef struct {
    char username[MAX_LINE];
    char password[MAX_LINE];
    int uid;
    int gid;
    char home_dir[MAX_LINE];
    char shell[MAX_LINE];
} VUser;

// System information
typedef struct {
    char hostname[MAX_LINE];
    char os_name[MAX_LINE];
    char kernel_version[MAX_LINE];
    long total_memory;
    long free_memory;
    int cpu_count;
    time_t boot_time;
} SystemInfo;

// Job structure
typedef struct {
    pid_t pid;
    char cmd[MAX_LINE];
    int status;  // 0: running, 1: stopped, 2: terminated
    int job_id;
} Job;

// Process structure
typedef struct {
    int pid;
    char name[MAX_LINE];
    int status;  // 0: running, 1: stopped, 2: terminated
    int priority;
    long memory_usage;
    time_t start_time;
    char owner[MAX_LINE];
} VProcess;

// Memory block structure
typedef struct {
    int id;
    char process_name[MAX_LINE];
    size_t size;
    void *address;
    int is_allocated;
} MemoryBlock;

// Network interface structure
typedef struct {
    char name[MAX_LINE];
    char ip_address[16];
    char mac_address[18];
    int is_up;
    long bytes_sent;
    long bytes_received;
} NetworkInterface;

// Device structure
typedef struct {
    char name[MAX_LINE];
    char type[MAX_LINE];
    int status;  // 0: offline, 1: online
    char driver[MAX_LINE];
    long last_access;
} Device;

// Global variables
char *history[MAX_HISTORY];
int history_count = 0;
int current_history = 0;
Job jobs[MAX_JOBS];
int job_count = 0;
int next_job_id = 1;

// Virtual environment variables
VFile *vfs_files[MAX_FILES];
VDirectory *vfs_dirs[MAX_DIRS];
VUser *users[MAX_USERS];
SystemInfo sys_info;
int current_user_id = 0;
char current_dir[MAX_LINE] = "/";

// Terminal settings
struct termios orig_termios;

// New global variables for virtual OS features
VProcess *processes[MAX_PROCESSES];
MemoryBlock *memory_blocks[MAX_MEMORY_BLOCKS];
NetworkInterface *network_interfaces[MAX_NETWORK_INTERFACES];
Device *devices[MAX_DEVICES];
int process_count = 0;
int memory_block_count = 0;
int network_interface_count = 0;
int device_count = 0;
int next_pid = 1;

// Function prototypes
void parse_command(char *line, char **args);
void execute_command(char **args);
void print_prompt();
void handle_cd(char **args);
void handle_echo(char **args);
void add_to_history(const char *line);
void load_history();
void save_history();
void handle_io_redirection(char **args, int *in_fd, int *out_fd);
void setup_terminal();
void restore_terminal();
char *get_history_entry(int direction);
void handle_jobs();
void handle_fg(char **args);
void handle_bg(char **args);
void handle_kill(char **args);
void check_jobs();
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void init_virtual_env();
void handle_vfs_command(char **args);
void create_vfile(const char *name, const char *content);
void create_vdir(const char *name);
void list_vfs();
void show_system_info();
void handle_user_command(char **args);
void init_system_info();
void update_system_info();
void init_process_management();
void init_memory_management();
void init_network_interfaces();
void init_devices();
void create_process(const char *name, int priority);
void terminate_process(int pid);
void allocate_memory(const char *process_name, size_t size);
void free_memory(int block_id);
void show_processes();
void show_memory_usage();
void show_network_status();
void show_devices();
void update_network_stats();
void update_device_status();
void handle_process_command(char **args);
void handle_memory_command(char **args);
void handle_network_command(char **args);
void handle_device_command(char **args);

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    int should_run = 1;

    // Initialize virtual environment
    init_virtual_env();
    init_system_info();

    // Set up signal handlers
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    // Load command history
    load_history();

    // Setup terminal
    setup_terminal();

    printf("Welcome to VirtualShell OS!\n");
    printf("Type 'help' for available commands\n");

    while (should_run) {
        print_prompt();
        
        // Read command line with history support
        int pos = 0;
        int c;
        while ((c = getchar()) != '\n' && pos < MAX_LINE - 1) {
            if (c == 27) { // Escape sequence
                getchar(); // Skip [
                c = getchar();
                if (c == 'A') { // Up arrow
                    char *prev = get_history_entry(1);
                    if (prev) {
                        printf("\r\033[K"); // Clear line
                        strcpy(line, prev);
                        pos = strlen(line);
                        printf("%s", line);
                    }
                } else if (c == 'B') { // Down arrow
                    char *next = get_history_entry(-1);
                    if (next) {
                        printf("\r\033[K"); // Clear line
                        strcpy(line, next);
                        pos = strlen(line);
                        printf("%s", line);
                    }
                }
            } else if (c == 127 || c == 8) { // Backspace
                if (pos > 0) {
                    printf("\b \b");
                    pos--;
                }
            } else {
                line[pos++] = c;
                putchar(c);
            }
        }
        line[pos] = '\0';
        printf("\n");

        // Skip empty lines
        if (strlen(line) == 0) continue;

        // Add to history
        add_to_history(line);

        // Parse command line
        parse_command(line, args);

        // Check for exit command
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        // Check for help command
        if (strcmp(args[0], "help") == 0) {
            printf("Available commands:\n");
            printf("  cd [directory]    - Change directory\n");
            printf("  pwd              - Print working directory\n");
            printf("  ls [options]     - List directory contents\n");
            printf("  echo [text]      - Print text\n");
            printf("  history          - Show command history\n");
            printf("  jobs             - List background jobs\n");
            printf("  fg [job_id]      - Bring job to foreground\n");
            printf("  bg [job_id]      - Continue job in background\n");
            printf("  kill [job_id]    - Kill a job\n");
            printf("  vfs              - Virtual filesystem commands\n");
            printf("  sysinfo          - Show system information\n");
            printf("  user             - User management commands\n");
            printf("  process          - Process management commands\n");
            printf("  memory           - Memory management commands\n");
            printf("  network          - Network interface commands\n");
            printf("  device           - Device management commands\n");
            printf("  help             - Show this help message\n");
            printf("  exit             - Exit the shell\n");
            continue;
        }

        // Check for virtual environment commands
        if (strcmp(args[0], "vfs") == 0) {
            handle_vfs_command(args);
            continue;
        }

        if (strcmp(args[0], "sysinfo") == 0) {
            show_system_info();
            continue;
        }

        if (strcmp(args[0], "user") == 0) {
            handle_user_command(args);
            continue;
        }

        // Check for built-in commands
        if (strcmp(args[0], "history") == 0) {
            for (int i = 0; i < history_count; i++) {
                printf("%d: %s\n", i + 1, history[i]);
            }
            continue;
        }

        if (strcmp(args[0], "jobs") == 0) {
            handle_jobs();
            continue;
        }

        if (strcmp(args[0], "fg") == 0) {
            handle_fg(args);
            continue;
        }

        if (strcmp(args[0], "bg") == 0) {
            handle_bg(args);
            continue;
        }

        if (strcmp(args[0], "kill") == 0) {
            handle_kill(args);
            continue;
        }

        // Add new command handlers
        if (strcmp(args[0], "process") == 0) {
            handle_process_command(args);
            continue;
        }

        if (strcmp(args[0], "memory") == 0) {
            handle_memory_command(args);
            continue;
        }

        if (strcmp(args[0], "network") == 0) {
            handle_network_command(args);
            continue;
        }

        if (strcmp(args[0], "device") == 0) {
            handle_device_command(args);
            continue;
        }

        // Execute command
        execute_command(args);
    }

    // Save history before exiting
    save_history();
    restore_terminal();
    printf("Goodbye!\n");
    return 0;
}

void setup_terminal() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void add_to_history(const char *line) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(line);
    } else {
        free(history[0]);
        for (int i = 0; i < history_count - 1; i++) {
            history[i] = history[i + 1];
        }
        history[history_count - 1] = strdup(line);
    }
    current_history = history_count;
}

void load_history() {
    FILE *file = fopen(HISTORY_FILE, "r");
    if (file) {
        char line[MAX_LINE];
        while (fgets(line, MAX_LINE, file) && history_count < MAX_HISTORY) {
            line[strcspn(line, "\n")] = 0;
            history[history_count++] = strdup(line);
        }
        fclose(file);
    }
    current_history = history_count;
}

void save_history() {
    FILE *file = fopen(HISTORY_FILE, "w");
    if (file) {
        for (int i = 0; i < history_count; i++) {
            fprintf(file, "%s\n", history[i]);
        }
        fclose(file);
    }
}

char *get_history_entry(int direction) {
    if (direction > 0 && current_history > 0) {
        return history[--current_history];
    } else if (direction < 0 && current_history < history_count - 1) {
        return history[++current_history];
    }
    return NULL;
}

void handle_io_redirection(char **args, int *in_fd, int *out_fd) {
    *in_fd = STDIN_FILENO;
    *out_fd = STDOUT_FILENO;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] != NULL) {
                *out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                args[i] = NULL;
                args[i + 1] = NULL;
            }
        } else if (strcmp(args[i], ">>") == 0) {
            if (args[i + 1] != NULL) {
                *out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                args[i] = NULL;
                args[i + 1] = NULL;
            }
        } else if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] != NULL) {
                *in_fd = open(args[i + 1], O_RDONLY);
                args[i] = NULL;
                args[i + 1] = NULL;
            }
        }
    }
}

void execute_command(char **args) {
    if (args[0] == NULL) return;

    // Handle built-in commands
    if (strcmp(args[0], "cd") == 0) {
        handle_cd(args);
        return;
    }

    if (strcmp(args[0], "echo") == 0) {
        handle_echo(args);
        return;
    }

    // Check if command should run in background
    int background = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            background = 1;
            args[i] = NULL;
            break;
        }
    }

    // Handle I/O redirection
    int in_fd, out_fd;
    handle_io_redirection(args, &in_fd, &out_fd);

    // Fork a child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
    } else if (pid == 0) {
        // Child process
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        if (execvp(args[0], args) < 0) {
            printf("Command not found: %s\n", args[0]);
            exit(1);
        }
    } else {
        // Parent process
        if (in_fd != STDIN_FILENO) close(in_fd);
        if (out_fd != STDOUT_FILENO) close(out_fd);

        if (background) {
            // Add to jobs list
            if (job_count < MAX_JOBS) {
                jobs[job_count].pid = pid;
                jobs[job_count].status = 0;
                jobs[job_count].job_id = next_job_id++;
                strncpy(jobs[job_count].cmd, args[0], MAX_LINE - 1);
                jobs[job_count].cmd[MAX_LINE - 1] = '\0';
                printf("[%d] %d\n", jobs[job_count].job_id, pid);
                job_count++;
            }
        } else {
            // Wait for foreground process
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

void print_prompt() {
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\033[1;32m%s\033[0m$ ", cwd);  // Green prompt
    } else {
        printf("$ ");
    }
    fflush(stdout);
}

void parse_command(char *line, char **args) {
    char *token;
    int i = 0;

    // Split line into tokens
    token = strtok(line, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;  // Null terminate the argument list
}

void handle_cd(char **args) {
    if (args[1] == NULL) {
        // No directory specified, go to home
        char *home = getenv("HOME");
        if (home == NULL) {
            printf("HOME environment variable not set\n");
            return;
        }
        if (chdir(home) != 0) {
            perror("cd");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
}

void handle_echo(char **args) {
    int i = 1;
    while (args[i] != NULL) {
        printf("%s ", args[i]);
        i++;
    }
    printf("\n");
}

void handle_jobs() {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].status == 0) {
            printf("[%d] Running %s\n", jobs[i].job_id, jobs[i].cmd);
        } else if (jobs[i].status == 1) {
            printf("[%d] Stopped %s\n", jobs[i].job_id, jobs[i].cmd);
        }
    }
}

void handle_fg(char **args) {
    if (args[1] == NULL) {
        printf("Usage: fg [job_id]\n");
        return;
    }

    int job_id = atoi(args[1]);
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            if (jobs[i].status == 1) {
                kill(jobs[i].pid, SIGCONT);
            }
            int status;
            waitpid(jobs[i].pid, &status, 0);
            // Remove job from list
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            return;
        }
    }
    printf("Job %d not found\n", job_id);
}

void handle_bg(char **args) {
    if (args[1] == NULL) {
        printf("Usage: bg [job_id]\n");
        return;
    }

    int job_id = atoi(args[1]);
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            if (jobs[i].status == 1) {
                kill(jobs[i].pid, SIGCONT);
                jobs[i].status = 0;
                printf("[%d] %s\n", jobs[i].job_id, jobs[i].cmd);
            }
            return;
        }
    }
    printf("Job %d not found\n", job_id);
}

void handle_kill(char **args) {
    if (args[1] == NULL) {
        printf("Usage: kill [job_id]\n");
        return;
    }

    int job_id = atoi(args[1]);
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            kill(jobs[i].pid, SIGTERM);
            // Remove job from list
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            return;
        }
    }
    printf("Job %d not found\n", job_id);
}

void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Remove terminated job from list
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                for (int j = i; j < job_count - 1; j++) {
                    jobs[j] = jobs[j + 1];
                }
                job_count--;
                break;
            }
        }
    }
}

void sigint_handler(int sig) {
    printf("\n");
    print_prompt();
}

void sigtstp_handler(int sig) {
    printf("\n");
    print_prompt();
}

void init_virtual_env() {
    // Initialize root directory
    create_vdir("/");
    
    // Create some default directories
    create_vdir("/bin");
    create_vdir("/home");
    create_vdir("/etc");
    
    // Create some default files
    create_vfile("/etc/passwd", "root:x:0:0:root:/root:/bin/sh\n");
    create_vfile("/etc/hostname", "virtualshell\n");
    
    // Initialize default user
    users[0] = malloc(sizeof(VUser));
    strcpy(users[0]->username, "root");
    strcpy(users[0]->password, "root");
    users[0]->uid = 0;
    users[0]->gid = 0;
    strcpy(users[0]->home_dir, "/root");
    strcpy(users[0]->shell, "/bin/sh");

    // Initialize new virtual OS components
    init_process_management();
    init_memory_management();
    init_network_interfaces();
    init_devices();
}

void init_system_info() {
    // Set up system information
    strcpy(sys_info.hostname, "virtualshell");
    strcpy(sys_info.os_name, "VirtualShell OS");
    strcpy(sys_info.kernel_version, "1.0.0");
    sys_info.total_memory = 1024 * 1024 * 1024; // 1GB
    sys_info.free_memory = 512 * 1024 * 1024;   // 512MB
    sys_info.cpu_count = 4;
    sys_info.boot_time = time(NULL);
}

void show_system_info() {
    update_system_info();
    printf("\nSystem Information:\n");
    printf("Hostname: %s\n", sys_info.hostname);
    printf("OS: %s\n", sys_info.os_name);
    printf("Kernel: %s\n", sys_info.kernel_version);
    printf("Memory: %ld MB total, %ld MB free\n", 
           sys_info.total_memory / (1024*1024),
           sys_info.free_memory / (1024*1024));
    printf("CPU: %d cores\n", sys_info.cpu_count);
    printf("Uptime: %ld seconds\n", time(NULL) - sys_info.boot_time);
}

void update_system_info() {
    // Simulate some system changes
    sys_info.free_memory = rand() % sys_info.total_memory;
}

void create_vfile(const char *name, const char *content) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs_files[i] == NULL) {
            vfs_files[i] = malloc(sizeof(VFile));
            strcpy(vfs_files[i]->name, name);
            strcpy(vfs_files[i]->content, content);
            vfs_files[i]->size = strlen(content);
            strcpy(vfs_files[i]->owner, "root");
            strcpy(vfs_files[i]->permissions, "rw-r--r--");
            vfs_files[i]->created = time(NULL);
            vfs_files[i]->modified = time(NULL);
            break;
        }
    }
}

void create_vdir(const char *name) {
    for (int i = 0; i < MAX_DIRS; i++) {
        if (vfs_dirs[i] == NULL) {
            vfs_dirs[i] = malloc(sizeof(VDirectory));
            strcpy(vfs_dirs[i]->name, name);
            vfs_dirs[i]->file_count = 0;
            strcpy(vfs_dirs[i]->owner, "root");
            strcpy(vfs_dirs[i]->permissions, "rwxr-xr-x");
            vfs_dirs[i]->created = time(NULL);
            break;
        }
    }
}

void list_vfs() {
    printf("\nVirtual Filesystem Contents:\n");
    printf("Directories:\n");
    for (int i = 0; i < MAX_DIRS; i++) {
        if (vfs_dirs[i] != NULL) {
            printf("  %s/  %s  %s\n", 
                   vfs_dirs[i]->name,
                   vfs_dirs[i]->permissions,
                   vfs_dirs[i]->owner);
        }
    }
    
    printf("\nFiles:\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs_files[i] != NULL) {
            printf("  %s  %s  %s  %d bytes\n",
                   vfs_files[i]->name,
                   vfs_files[i]->permissions,
                   vfs_files[i]->owner,
                   vfs_files[i]->size);
        }
    }
}

void handle_vfs_command(char **args) {
    if (args[1] == NULL) {
        printf("Usage: vfs [list|create|delete|read|write]\n");
        return;
    }

    if (strcmp(args[1], "list") == 0) {
        list_vfs();
    } else if (strcmp(args[1], "create") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            printf("Usage: vfs create <name> <content>\n");
            return;
        }
        create_vfile(args[2], args[3]);
    } else {
        printf("Unknown vfs command: %s\n", args[1]);
    }
}

void handle_user_command(char **args) {
    if (args[1] == NULL) {
        printf("Usage: user [list|add|del|info]\n");
        return;
    }

    if (strcmp(args[1], "list") == 0) {
        printf("\nUsers:\n");
        for (int i = 0; i < MAX_USERS; i++) {
            if (users[i] != NULL) {
                printf("  %s (uid=%d, gid=%d)\n",
                       users[i]->username,
                       users[i]->uid,
                       users[i]->gid);
            }
        }
    } else if (strcmp(args[1], "info") == 0) {
        printf("\nCurrent User Information:\n");
        if (users[current_user_id] != NULL) {
            printf("Username: %s\n", users[current_user_id]->username);
            printf("UID: %d\n", users[current_user_id]->uid);
            printf("GID: %d\n", users[current_user_id]->gid);
            printf("Home: %s\n", users[current_user_id]->home_dir);
            printf("Shell: %s\n", users[current_user_id]->shell);
        }
    } else {
        printf("Unknown user command: %s\n", args[1]);
    }
}

void init_process_management() {
    // Create initial system processes
    create_process("init", 0);
    create_process("systemd", 0);
    create_process("kernel", 0);
}

void init_memory_management() {
    // Initialize memory blocks
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        memory_blocks[i] = malloc(sizeof(MemoryBlock));
        memory_blocks[i]->is_allocated = 0;
    }
}

void init_network_interfaces() {
    // Initialize network interfaces
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        network_interfaces[i] = malloc(sizeof(NetworkInterface));
        sprintf(network_interfaces[i]->name, "eth%d", i);
        sprintf(network_interfaces[i]->ip_address, "192.168.1.%d", i + 1);
        sprintf(network_interfaces[i]->mac_address, "00:00:00:00:00:%02x", i + 1);
        network_interfaces[i]->is_up = 1;
        network_interfaces[i]->bytes_sent = 0;
        network_interfaces[i]->bytes_received = 0;
        network_interface_count++;
    }
}

void init_devices() {
    // Initialize some basic devices
    devices[0] = malloc(sizeof(Device));
    strcpy(devices[0]->name, "sda");
    strcpy(devices[0]->type, "disk");
    devices[0]->status = 1;
    strcpy(devices[0]->driver, "scsi");
    devices[0]->last_access = time(NULL);
    device_count++;

    devices[1] = malloc(sizeof(Device));
    strcpy(devices[1]->name, "tty0");
    strcpy(devices[1]->type, "terminal");
    devices[1]->status = 1;
    strcpy(devices[1]->driver, "tty");
    devices[1]->last_access = time(NULL);
    device_count++;
}

void create_process(const char *name, int priority) {
    if (process_count < MAX_PROCESSES) {
        processes[process_count] = malloc(sizeof(VProcess));
        processes[process_count]->pid = next_pid++;
        strcpy(processes[process_count]->name, name);
        processes[process_count]->status = 0;
        processes[process_count]->priority = priority;
        processes[process_count]->memory_usage = 0;
        processes[process_count]->start_time = time(NULL);
        strcpy(processes[process_count]->owner, "root");
        process_count++;
    }
}

void terminate_process(int pid) {
    for (int i = 0; i < process_count; i++) {
        if (processes[i]->pid == pid) {
            processes[i]->status = 2;
            // Free associated memory
            for (int j = 0; j < memory_block_count; j++) {
                if (strcmp(memory_blocks[j]->process_name, processes[i]->name) == 0) {
                    free_memory(memory_blocks[j]->id);
                }
            }
            break;
        }
    }
}

void allocate_memory(const char *process_name, size_t size) {
    if (memory_block_count < MAX_MEMORY_BLOCKS) {
        memory_blocks[memory_block_count]->id = memory_block_count;
        strcpy(memory_blocks[memory_block_count]->process_name, process_name);
        memory_blocks[memory_block_count]->size = size;
        memory_blocks[memory_block_count]->is_allocated = 1;
        memory_block_count++;
    }
}

void free_memory(int block_id) {
    if (block_id >= 0 && block_id < memory_block_count) {
        memory_blocks[block_id]->is_allocated = 0;
        memory_blocks[block_id]->size = 0;
    }
}

void show_processes() {
    printf("\nProcess List:\n");
    printf("PID\tName\t\tStatus\tPriority\tMemory\tOwner\n");
    for (int i = 0; i < process_count; i++) {
        const char *status_str = processes[i]->status == 0 ? "Running" :
                               processes[i]->status == 1 ? "Stopped" : "Terminated";
        printf("%d\t%s\t\t%s\t%d\t\t%ld\t%s\n",
               processes[i]->pid,
               processes[i]->name,
               status_str,
               processes[i]->priority,
               processes[i]->memory_usage,
               processes[i]->owner);
    }
}

void show_memory_usage() {
    printf("\nMemory Usage:\n");
    printf("Block ID\tProcess\t\tSize\tStatus\n");
    for (int i = 0; i < memory_block_count; i++) {
        if (memory_blocks[i]->is_allocated) {
            printf("%d\t\t%s\t\t%zu\tAllocated\n",
                   memory_blocks[i]->id,
                   memory_blocks[i]->process_name,
                   memory_blocks[i]->size);
        }
    }
}

void show_network_status() {
    printf("\nNetwork Interfaces:\n");
    printf("Interface\tIP Address\t\tMAC Address\t\tStatus\tSent\tReceived\n");
    for (int i = 0; i < network_interface_count; i++) {
        printf("%s\t\t%s\t\t%s\t%s\t%ld\t%ld\n",
               network_interfaces[i]->name,
               network_interfaces[i]->ip_address,
               network_interfaces[i]->mac_address,
               network_interfaces[i]->is_up ? "UP" : "DOWN",
               network_interfaces[i]->bytes_sent,
               network_interfaces[i]->bytes_received);
    }
}

void show_devices() {
    printf("\nDevices:\n");
    printf("Name\tType\t\tStatus\tDriver\t\tLast Access\n");
    for (int i = 0; i < device_count; i++) {
        printf("%s\t%s\t\t%s\t%s\t\t%s",
               devices[i]->name,
               devices[i]->type,
               devices[i]->status ? "Online" : "Offline",
               devices[i]->driver,
               ctime(&devices[i]->last_access));
    }
}

void update_network_stats() {
    for (int i = 0; i < network_interface_count; i++) {
        if (network_interfaces[i]->is_up) {
            network_interfaces[i]->bytes_sent += rand() % 1000;
            network_interfaces[i]->bytes_received += rand() % 1000;
        }
    }
}

void update_device_status() {
    for (int i = 0; i < device_count; i++) {
        if (rand() % 100 < 5) {  // 5% chance of status change
            devices[i]->status = !devices[i]->status;
        }
        if (devices[i]->status) {
            devices[i]->last_access = time(NULL);
        }
    }
}

void handle_process_command(char **args) {
    if (args[1] == NULL) {
        printf("Usage: process [list|create|kill|info]\n");
        return;
    }

    if (strcmp(args[1], "list") == 0) {
        show_processes();
    } else if (strcmp(args[1], "create") == 0) {
        if (args[2] == NULL) {
            printf("Usage: process create <name> [priority]\n");
            return;
        }
        int priority = args[3] ? atoi(args[3]) : 0;
        create_process(args[2], priority);
    } else if (strcmp(args[1], "kill") == 0) {
        if (args[2] == NULL) {
            printf("Usage: process kill <pid>\n");
            return;
        }
        terminate_process(atoi(args[2]));
    }
}

void handle_memory_command(char **args) {
    if (args[1] == NULL) {
        printf("Usage: memory [list|alloc|free]\n");
        return;
    }

    if (strcmp(args[1], "list") == 0) {
        show_memory_usage();
    } else if (strcmp(args[1], "alloc") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            printf("Usage: memory alloc <process> <size>\n");
            return;
        }
        allocate_memory(args[2], atoi(args[3]));
    } else if (strcmp(args[1], "free") == 0) {
        if (args[2] == NULL) {
            printf("Usage: memory free <block_id>\n");
            return;
        }
        free_memory(atoi(args[2]));
    }
}

void handle_network_command(char **args) {
    if (args[1] == NULL) {
        printf("Usage: network [list|up|down|stats]\n");
        return;
    }

    if (strcmp(args[1], "list") == 0) {
        show_network_status();
    } else if (strcmp(args[1], "stats") == 0) {
        update_network_stats();
        show_network_status();
    }
}

void handle_device_command(char **args) {
    if (args[1] == NULL) {
        printf("Usage: device [list|status]\n");
        return;
    }

    if (strcmp(args[1], "list") == 0) {
        show_devices();
    } else if (strcmp(args[1], "status") == 0) {
        update_device_status();
        show_devices();
    }
} 