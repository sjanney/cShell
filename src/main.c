#include "../include/shell/shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define CSHELL_VERSION "1.0"

// Signal handler for graceful shutdown
static void signal_handler(int signum) {
    printf("\nReceived signal %d, shutting down...\n", signum);
    shell_cleanup();
    exit(0);
}

int main(int argc, char *argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);

    // Initialize shell
    if (shell_init() != 0) {
        fprintf(stderr, "Failed to initialize shell\n");
        return 1;
    }

    // Run the shell
    int status = shell_run();

    // Cleanup
    shell_cleanup();

    return status;
} 