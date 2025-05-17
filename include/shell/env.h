#ifndef ENV_H
#define ENV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define ENV_MAX_NAME 64
#define ENV_MAX_VALUE 1024
#define ENV_MAX_VARS 256

typedef struct {
    char name[ENV_MAX_NAME];
    char value[ENV_MAX_VALUE];
} EnvVar;

// Environment initialization and cleanup
int env_init(void);
void env_cleanup(void);

// Environment operations
int env_set(const char *name, const char *value);
char *env_get(const char *name);
int env_unset(const char *name);
char **env_list(int *count);

// Environment utility functions
char *env_expand(const char *str);
int env_export(const char *name);
int env_import_from_host(void);
int env_save_to_file(const char *path);
int env_load_from_file(const char *path);

// New function declaration
bool env_exists(const char *name);
char **env_get_all(int *count);

#endif // ENV_H 