#ifndef CSHELL_AI_H
#define CSHELL_AI_H

#include <stdbool.h>

// AI command types
typedef enum {
    AI_CMD_UNKNOWN,
    AI_CMD_HELP,
    AI_CMD_EXPLAIN,
    AI_CMD_SUGGEST,
    AI_CMD_EXECUTE,
    AI_CMD_LEARN
} AICommandType;

// AI response structure
typedef struct {
    bool success;
    char *message;
    char *suggestion;
    char *command;
} AIResponse;

// Initialize AI module
int ai_init(void);

// Clean up AI module
void ai_cleanup(void);

// Process natural language input
AIResponse ai_process_input(const char *input);

// Get AI command type from input
AICommandType ai_get_command_type(const char *input);

// Generate command suggestion
char *ai_suggest_command(const char *description);

// Explain a command
char *ai_explain_command(const char *command);

// Learn from user feedback
void ai_learn(const char *input, const char *feedback);

// Get AI status
bool ai_is_available(void);

#endif // CSHELL_AI_H 