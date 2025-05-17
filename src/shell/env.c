#include "../../include/shell/env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>

// Global environment variables
static EnvVar env_vars[ENV_MAX_VARS];
static int env_count = 0;

// Initialize environment variables
int env_init(void) {
    // Clear environment array
    memset(env_vars, 0, sizeof(env_vars));
    env_count = 0;
    
    // Set basic environment variables
    struct passwd *pw = getpwuid(getuid());
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    env_set("PATH", "/bin:/usr/bin");
    env_set("HOME", pw ? pw->pw_dir : "/home");
    env_set("USER", pw ? pw->pw_name : "user");
    env_set("HOSTNAME", hostname);
    env_set("PWD", "/");
    env_set("SHELL", "/bin/cshell");
    env_set("TERM", "xterm-256color");
    env_set("PS1", "\\u@\\h:\\w\\$ ");
    env_set("CSHELL_VERSION", "1.0.0");
    
    // Load system environment variables
    extern char **environ;
    if (environ) {
        for (char **env = environ; *env != NULL; env++) {
            char *equals = strchr(*env, '=');
            if (equals) {
                *equals = '\0';
                env_set(*env, equals + 1);
                *equals = '=';
            }
        }
    }
    
    return 0;
}

// Clean up environment variables
void env_cleanup(void) {
    // Nothing to do for this simple implementation
    env_count = 0;
}

// Set an environment variable
int env_set(const char *name, const char *value) {
    if (!name || !value) {
        return -1;
    }
    
    // Check name length
    if (strlen(name) >= ENV_MAX_NAME) {
        return -1;
    }
    
    // Check value length
    if (strlen(value) >= ENV_MAX_VALUE) {
        return -1;
    }
    
    // Check if variable already exists
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            // Update existing variable
            strncpy(env_vars[i].value, value, ENV_MAX_VALUE - 1);
            return 0;
        }
    }
    
    // Check if we have room for a new variable
    if (env_count >= ENV_MAX_VARS) {
        return -1;
    }
    
    // Add new variable
    strncpy(env_vars[env_count].name, name, ENV_MAX_NAME - 1);
    strncpy(env_vars[env_count].value, value, ENV_MAX_VALUE - 1);
    env_count++;
    
    return 0;
}

// Get an environment variable
char *env_get(const char *name) {
    if (!name) {
        return NULL;
    }
    
    // Look for variable
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            return env_vars[i].value;
        }
    }
    
    return NULL;
}

// Unset an environment variable
int env_unset(const char *name) {
    if (!name) {
        return -1;
    }
    
    // Look for variable
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            // Move all variables after this one up
            for (int j = i; j < env_count - 1; j++) {
                strncpy(env_vars[j].name, env_vars[j + 1].name, ENV_MAX_NAME);
                strncpy(env_vars[j].value, env_vars[j + 1].value, ENV_MAX_VALUE);
            }
            env_count--;
            return 0;
        }
    }
    
    return -1;
}

// List all environment variables
char **env_list(int *count) {
    if (!count) {
        return NULL;
    }
    
    *count = env_count;
    if (*count == 0) {
        return NULL;
    }
    
    // Allocate array for variable pointers
    char **list = (char **)malloc(env_count * sizeof(char *));
    if (!list) {
        return NULL;
    }
    
    // Allocate strings for each variable
    for (int i = 0; i < env_count; i++) {
        list[i] = (char *)malloc(ENV_MAX_NAME + ENV_MAX_VALUE + 2);
        if (!list[i]) {
            // Free allocations so far
            for (int j = 0; j < i; j++) {
                free(list[j]);
            }
            free(list);
            return NULL;
        }
        
        // Format variable as NAME=VALUE
        snprintf(list[i], ENV_MAX_NAME + ENV_MAX_VALUE + 2, 
                 "%s=%s", env_vars[i].name, env_vars[i].value);
    }
    
    return list;
}

// Expand environment variables in a string
char *env_expand(const char *str) {
    if (!str) {
        return NULL;
    }
    
    // Allocate buffer for result (overestimate size)
    char *result = (char *)malloc(strlen(str) * 2);
    if (!result) {
        return NULL;
    }
    
    const char *p = str;
    char *r = result;
    
    while (*p) {
        if (*p == '$' && *(p+1) == '{') {
            // Found ${VAR} format
            p += 2;
            const char *end = strchr(p, '}');
            if (!end) {
                // No closing brace, copy as-is
                *r++ = '$';
                *r++ = '{';
                continue;
            }
            
            // Extract variable name
            int name_len = end - p;
            char name[ENV_MAX_NAME];
            if (name_len >= ENV_MAX_NAME) {
                name_len = ENV_MAX_NAME - 1;
            }
            strncpy(name, p, name_len);
            name[name_len] = '\0';
            
            // Get variable value
            char *value = env_get(name);
            if (value) {
                // Copy value to result
                strcpy(r, value);
                r += strlen(value);
            }
            
            // Skip past variable
            p = end + 1;
        } else if (*p == '$' && (isalpha(*(p+1)) || *(p+1) == '_')) {
            // Found $VAR format
            p++;
            const char *start = p;
            
            // Find end of variable name
            while (isalnum(*p) || *p == '_') {
                p++;
            }
            
            // Extract variable name
            int name_len = p - start;
            char name[ENV_MAX_NAME];
            if (name_len >= ENV_MAX_NAME) {
                name_len = ENV_MAX_NAME - 1;
            }
            strncpy(name, start, name_len);
            name[name_len] = '\0';
            
            // Get variable value
            char *value = env_get(name);
            if (value) {
                // Copy value to result
                strcpy(r, value);
                r += strlen(value);
            }
        } else {
            // Copy character as-is
            *r++ = *p++;
        }
    }
    
    // Null-terminate result
    *r = '\0';
    
    return result;
}

// Export an environment variable to the host system
int env_export(const char *name) {
    if (!name) {
        return -1;
    }
    
    // Find variable
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            // Export to host environment
            setenv(name, env_vars[i].value, 1);
            return 0;
        }
    }
    
    return -1;
}

// Import environment variables from host system
int env_import_from_host(void) {
    extern char **environ;
    int imported = 0;
    
    // Loop through host environment
    for (char **env = environ; *env; env++) {
        // Split into name and value
        char *eq = strchr(*env, '=');
        if (!eq) {
            continue;
        }
        
        // Extract name
        int name_len = eq - *env;
        if (name_len >= ENV_MAX_NAME) {
            continue;
        }
        
        char name[ENV_MAX_NAME];
        strncpy(name, *env, name_len);
        name[name_len] = '\0';
        
        // Extract value
        const char *value = eq + 1;
        
        // Set in our environment
        if (env_set(name, value) == 0) {
            imported++;
        }
    }
    
    return imported;
}

// Save environment to a file
int env_save_to_file(const char *path) {
    if (!path) {
        return -1;
    }
    
    // Open file for writing
    FILE *file = fopen(path, "w");
    if (!file) {
        return -1;
    }
    
    // Write variables to file
    for (int i = 0; i < env_count; i++) {
        fprintf(file, "%s=%s\n", env_vars[i].name, env_vars[i].value);
    }
    
    fclose(file);
    return env_count;
}

// Load environment from a file
int env_load_from_file(const char *path) {
    if (!path) {
        return -1;
    }
    
    // Open file for reading
    FILE *file = fopen(path, "r");
    if (!file) {
        return -1;
    }
    
    char line[ENV_MAX_NAME + ENV_MAX_VALUE + 2];
    int loaded = 0;
    
    // Read lines from file
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        char *nl = strchr(line, '\n');
        if (nl) {
            *nl = '\0';
        }
        
        // Split into name and value
        char *eq = strchr(line, '=');
        if (!eq) {
            continue;
        }
        
        // Extract name
        int name_len = eq - line;
        if (name_len >= ENV_MAX_NAME) {
            continue;
        }
        
        char name[ENV_MAX_NAME];
        strncpy(name, line, name_len);
        name[name_len] = '\0';
        
        // Extract value
        const char *value = eq + 1;
        
        // Set in our environment
        if (env_set(name, value) == 0) {
            loaded++;
        }
    }
    
    fclose(file);
    return loaded;
}

// Get all environment variables
char **env_get_all(int *count) {
    // Allocate memory for variable pointers
    char **env_list = (char **)malloc(sizeof(char *) * (env_count + 1));
    if (!env_list) {
        *count = 0;
        return NULL;
    }
    
    // Create formatted strings for each variable
    for (int i = 0; i < env_count; i++) {
        size_t len = strlen(env_vars[i].name) + strlen(env_vars[i].value) + 2; // +2 for '=' and '\0'
        env_list[i] = (char *)malloc(len);
        if (!env_list[i]) {
            // Free previously allocated memory on error
            for (int j = 0; j < i; j++) {
                free(env_list[j]);
            }
            free(env_list);
            *count = 0;
            return NULL;
        }
        snprintf(env_list[i], len, "%s=%s", env_vars[i].name, env_vars[i].value);
    }
    
    *count = env_count;
    env_list[env_count] = NULL; // NULL terminate the list
    return env_list;
} 