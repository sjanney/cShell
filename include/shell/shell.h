#ifndef CSHELL_SHELL_H
#define CSHELL_SHELL_H

#include <stdbool.h>
#include "commands.h"

// Constants
#define SHELL_MAX_INPUT 1024
#define SHELL_MAX_PROMPT 128
#define SHELL_MAX_ARGS 64
#define SHELL_MAX_HISTORY 100

// Function declarations
int shell_init(void);
void shell_cleanup(void);
int shell_run(void);
int shell_parse_and_execute(char *input);
int shell_execute_command(int argc, char **argv);
void shell_display_prompt(void);
char *shell_read_line(void);
void shell_add_to_history(const char *input);
void shell_clear_history(void);
char *shell_get_history_entry(int index);
void shell_show_history(void);
int shell_run_script(const char *filename);

#endif // CSHELL_SHELL_H 