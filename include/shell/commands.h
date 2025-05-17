#ifndef CSHELL_COMMANDS_H
#define CSHELL_COMMANDS_H

// Command structure
typedef struct {
    const char *name;
    const char *description;
    int (*func)(int argc, char **argv);
} Command;

// Basic commands
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

// Process management commands
int cmd_ps(int argc, char **argv);
int cmd_kill(int argc, char **argv);
int cmd_bg(int argc, char **argv);
int cmd_fg(int argc, char **argv);
int cmd_jobs(int argc, char **argv);

// Environment commands
int cmd_env(int argc, char **argv);
int cmd_export(int argc, char **argv);
int cmd_unset(int argc, char **argv);

// AI commands
int cmd_ai_help(int argc, char **argv);
int cmd_ai_explain(int argc, char **argv);
int cmd_ai_suggest(int argc, char **argv);
int cmd_ai_learn(int argc, char **argv);

// External command table
extern Command builtin_commands[];

#endif // CSHELL_COMMANDS_H 