#include "../../include/shell/shell.h"
#include "../../include/shell/commands.h"
#include "../../include/shell/env.h"
#include "../../include/shell/process.h"
#include "../../include/shell/ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>
#include <fcntl.h>
#include <ctype.h>

// Color definitions
#define COLOR_RESET     "\033[0m"
#define COLOR_BOLD      "\033[1m"
#define COLOR_RED       "\033[31m"
#define COLOR_GREEN     "\033[32m"
#define COLOR_YELLOW    "\033[33m"
#define COLOR_BLUE      "\033[34m"
#define COLOR_MAGENTA   "\033[35m"
#define COLOR_CYAN      "\033[36m"

// ASCII art logo
static const char *LOGO = 
"$$\\    $$\\ $$\\            $$\\                         $$\\  $$$$$$\\  $$\\                 $$\\ $$\\ \n"
"$$ |   $$ |\\__|           $$ |                        $$ |$$  __$$\\ $$ |                $$ |$$ |\n"
"$$ |   $$ |$$\\  $$$$$$\\ $$$$$$\\   $$\\   $$\\  $$$$$$\\  $$ |$$ /  \\__|$$$$$$$\\   $$$$$$\\  $$ |$$ |\n"
"\\$$\\  $$  |$$ |$$  __$$\\\\_$$  _|  $$ |  $$ | \\____$$\\ $$ |\\$$$$$$\\  $$  __$$\\ $$  __$$\\ $$ |$$ |\n"
" \\$$\\$$  / $$ |$$ |  \\__| $$ |    $$ |  $$ | $$$$$$$ |$$ | \\____$$\\ $$ |  $$ |$$$$$$$$ |$$ |$$ |\n"
"  \\$$$  /  $$ |$$ |       $$ |$$\\ $$ |  $$ |$$  __$$ |$$ |$$\\   $$ |$$ |  $$ |$$   ____|$$ |$$ |\n"
"   \\$  /   $$ |$$ |       \\$$$$  |\\$$$$$$  |\\$$$$$$$ |$$ |\\$$$$$$  |$$ |  $$ |\\$$$$$$$\\ $$ |$$ |\n"
"    \\_/    \\__|\\__|        \\____/  \\______/  \\_______|\\__| \\______/ \\__|  \\__| \\_______|\\__|\\__|\n"
"                                                                                                \n";

// Constants
#define MAX_PATH_LENGTH 1024
#define MAX_USERNAME_LENGTH 256
#define MAX_HOSTNAME_LENGTH 256

// Global shell variables
static char current_dir[MAX_PATH_LENGTH];
static char home_dir[MAX_PATH_LENGTH];
static char username[MAX_USERNAME_LENGTH];
static char hostname[MAX_HOSTNAME_LENGTH];
static char history[SHELL_MAX_HISTORY][SHELL_MAX_INPUT];
static int history_count = 0;
static int running = 0;

// Forward declarations
void shell_setup_signals(void);
void shell_parse_command(char *line, char **argv, int *argc);
void shell_handle_signal(int sig);

// Initialize the shell
int shell_init(void) {
    // Initialize environment variables
    if (env_init() != 0) {
        fprintf(stderr, "Failed to initialize environment\n");
        return 1;
    }
    
    // Initialize process management
    if (process_init() != 0) {
        fprintf(stderr, "Failed to initialize process management\n");
        env_cleanup();
        return 1;
    }
    
    // Initialize AI module
    if (ai_init() != 0) {
        fprintf(stderr, "Warning: Failed to initialize AI module\n");
    }
    
    // Get current directory
    if (getcwd(current_dir, MAX_PATH_LENGTH) == NULL) {
        strcpy(current_dir, "/");
    }
    
    // Get home directory
    char *home = getenv("HOME");
    if (home) {
        strncpy(home_dir, home, MAX_PATH_LENGTH - 1);
    } else {
        strncpy(home_dir, current_dir, MAX_PATH_LENGTH - 1);
    }
    
    // Get username
    char *user = getenv("USER");
    if (user) {
        strncpy(username, user, MAX_USERNAME_LENGTH - 1);
    } else {
        strncpy(username, "user", MAX_USERNAME_LENGTH - 1);
    }
    
    // Get hostname
    if (gethostname(hostname, MAX_HOSTNAME_LENGTH) != 0) {
        strncpy(hostname, "localhost", MAX_HOSTNAME_LENGTH - 1);
    }
    
    // Clear history
    memset(history, 0, sizeof(history));
    history_count = 0;
    
    // Set up signal handlers
    shell_setup_signals();
    
    running = 1;
    return 0;
}

// Clean up shell resources
void shell_cleanup(void) {
    ai_cleanup();
    process_cleanup();
    env_cleanup();
    running = 0;
}

// Main shell loop
int shell_run(void) {
    // Display welcome message
    printf("\n");
    printf(COLOR_CYAN COLOR_BOLD "%s" COLOR_RESET, LOGO);
    printf(COLOR_BOLD "Welcome to cShell v1.0 - A Modular Shell Environment\n" COLOR_RESET);
    printf(COLOR_YELLOW "==================================================" COLOR_RESET "\n");
    printf("Features:\n");
    printf("  - " COLOR_GREEN "Process Management" COLOR_RESET " (ps, jobs, fg, bg)\n");
    printf("  - " COLOR_GREEN "Environment Variables" COLOR_RESET " (env, export, unset)\n");
    printf("  - " COLOR_GREEN "AI Assistance" COLOR_RESET " (ai-help, ai-explain, ai-suggest)\n");
    printf("  - " COLOR_GREEN "File Operations" COLOR_RESET " (ls, cat, mkdir, touch)\n\n");
    printf("Type " COLOR_GREEN "help" COLOR_RESET " for a list of commands\n\n");
    
    char input[SHELL_MAX_INPUT];
    
    while (running) {
        // Display prompt
        shell_display_prompt();
        
        // Read input
        char *line = shell_read_line();
        if (!line) {
            break;
        }
        
        // Skip empty lines
        if (line[0] == '\0') {
            continue;
        }
        
        // Add to history
        shell_add_to_history(line);
        
        // Parse and execute command
        shell_parse_and_execute(line);
    }
    
    printf("\nGoodbye!\n");
    return 0;
}

// Parse and execute a command
int shell_parse_and_execute(char *input) {
    if (!input || input[0] == '\0') {
        return 0;
    }
    
    // Parse input into command and arguments
    char *argv[SHELL_MAX_ARGS];
    int argc = 0;
    
    shell_parse_command(input, argv, &argc);
    
    if (argc > 0) {
        return shell_execute_command(argc, argv);
    }
    
    return 0;
}

// Parse command line into arguments
void shell_parse_command(char *line, char **argv, int *argc) {
    *argc = 0;
    char *token = strtok(line, " \t\n");
    
    while (token != NULL && *argc < SHELL_MAX_ARGS - 1) {
        argv[(*argc)++] = token;
        token = strtok(NULL, " \t\n");
    }
    
    argv[*argc] = NULL;
}

// Execute a command
int shell_execute_command(int argc, char **argv) {
    if (argc == 0) {
        return 0;
    }
    
    // Check for built-in commands
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strcmp(argv[0], builtin_commands[i].name) == 0) {
            return builtin_commands[i].func(argc, argv);
        }
    }
    
    // Execute external command
    Process *process = process_create(argv[0], argv, argc, true);
    if (!process) {
        fprintf(stderr, COLOR_RED "Error: Command not found: %s\n" COLOR_RESET, argv[0]);
        return 1;
    }
    
    return 0;
}

// Display shell prompt
void shell_display_prompt(void) {
    char *pwd = current_dir;
    
    // Replace home directory with ~
    if (strncmp(current_dir, home_dir, strlen(home_dir)) == 0) {
        pwd = current_dir + strlen(home_dir);
        printf(COLOR_GREEN "%s@%s" COLOR_RESET ":" COLOR_BLUE "~%s" COLOR_RESET "$ ", 
               username, hostname, pwd);
    } else {
        printf(COLOR_GREEN "%s@%s" COLOR_RESET ":" COLOR_BLUE "%s" COLOR_RESET "$ ", 
               username, hostname, pwd);
    }
    
    fflush(stdout);
}

// Set up signal handlers
void shell_setup_signals(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = shell_handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

// Handle signals
void shell_handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\n");
        shell_display_prompt();
        fflush(stdout);
    } else if (sig == SIGTERM) {
        running = 0;
    }
}

// Read a line of input
char *shell_read_line(void) {
    static char buffer[SHELL_MAX_INPUT];
    
    if (fgets(buffer, SHELL_MAX_INPUT, stdin) == NULL) {
        return NULL;
    }
    
    // Remove trailing newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return buffer;
}

// Add a command to history
void shell_add_to_history(const char *input) {
    if (!input || input[0] == '\0') {
        return;
    }
    
    // Check if it's a duplicate of the last command
    if (history_count > 0 && strcmp(history[history_count - 1], input) == 0) {
        return;
    }
    
    // Add to history
    if (history_count < SHELL_MAX_HISTORY) {
        strncpy(history[history_count++], input, SHELL_MAX_INPUT - 1);
    } else {
        // Shift history down to make room
        for (int i = 1; i < SHELL_MAX_HISTORY; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strncpy(history[SHELL_MAX_HISTORY - 1], input, SHELL_MAX_INPUT - 1);
    }
}

// Clear history
void shell_clear_history(void) {
    memset(history, 0, sizeof(history));
    history_count = 0;
}

// Get a history entry
char *shell_get_history_entry(int index) {
    if (index < 0 || index >= history_count) {
        return NULL;
    }
    
    return history[index];
}

// Show command history
void shell_show_history(void) {
    for (int i = 0; i < history_count; i++) {
        printf("%3d  %s\n", i + 1, history[i]);
    }
}

// Run a script file
int shell_run_script(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return 1;
    }
    
    char line[SHELL_MAX_INPUT];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Remove comments
        char *comment = strchr(line, '#');
        if (comment) {
            *comment = '\0';
        }
        
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        // Skip empty lines
        if (line[0] == '\0') {
            continue;
        }
        
        // Execute command
        shell_parse_and_execute(line);
    }
    
    fclose(file);
    return 0;
} 