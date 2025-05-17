#include "../../include/shell/ai.h"
#include "../../include/shell/shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

// Color definitions
#define COLOR_RESET     "\033[0m"
#define COLOR_BOLD      "\033[1m"
#define COLOR_RED       "\033[31m"
#define COLOR_GREEN     "\033[32m"
#define COLOR_YELLOW    "\033[33m"
#define COLOR_BLUE      "\033[34m"
#define COLOR_MAGENTA   "\033[35m"
#define COLOR_CYAN      "\033[36m"

// Global variables
static bool ai_initialized = false;
static char *ai_api_key = NULL;
static char *ai_model = "gpt-3.5-turbo";
static char *ai_endpoint = "https://api.openai.com/v1/chat/completions";

// Response buffer structure
typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

// Write callback function for CURL
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    ResponseBuffer *resp = (ResponseBuffer *)stream;
    size_t new_size = resp->size + size * nmemb;
    resp->data = realloc(resp->data, new_size + 1);
    memcpy(resp->data + resp->size, ptr, size * nmemb);
    resp->data[new_size] = '\0';
    resp->size = new_size;
    return size * nmemb;
}

// Initialize AI module
int ai_init(void) {
    if (ai_initialized) {
        return 0;
    }

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Get API key from environment
    ai_api_key = getenv("OPENAI_API_KEY");
    if (!ai_api_key) {
        printf(COLOR_YELLOW "Warning: OPENAI_API_KEY not set. AI features will be limited.\n" COLOR_RESET);
    }

    ai_initialized = true;
    return 0;
}

// Clean up AI module
void ai_cleanup(void) {
    if (!ai_initialized) {
        return;
    }

    curl_global_cleanup();
    ai_initialized = false;
}

// Helper function to make API request
static char *make_api_request(const char *prompt) {
    if (!ai_api_key) {
        return strdup("AI API key not configured");
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        return strdup("Failed to initialize CURL");
    }

    // Prepare JSON payload
    char *json_payload;
    asprintf(&json_payload,
        "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}",
        ai_model, prompt);

    // Set up headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", ai_api_key);
    headers = curl_slist_append(headers, auth_header);

    // Set up response buffer
    ResponseBuffer response = {NULL, 0};

    // Set up CURL options
    curl_easy_setopt(curl, CURLOPT_URL, ai_endpoint);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Make request
    CURLcode res = curl_easy_perform(curl);
    char *result = NULL;

    if (res == CURLE_OK) {
        // Parse response (simplified)
        result = strdup(response.data);
    } else {
        result = strdup("Failed to make API request");
    }

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(json_payload);
    free(response.data);

    return result;
}

// Process natural language input
AIResponse ai_process_input(const char *input) {
    AIResponse response = {false, NULL, NULL, NULL};
    
    if (!ai_initialized) {
        response.message = strdup("AI module not initialized");
        return response;
    }

    // Prepare prompt for AI
    char *prompt;
    asprintf(&prompt, 
        "You are an AI assistant for a Unix-like shell. "
        "The user said: \"%s\". "
        "Respond with a JSON object containing: "
        "1. success (boolean), "
        "2. message (string explaining what to do), "
        "3. suggestion (string with command suggestion), "
        "4. command (string with exact command to execute). "
        "Keep responses concise and focused on shell operations.", 
        input);

    // Get AI response
    char *ai_response = make_api_request(prompt);
    free(prompt);

    if (ai_response) {
        // Parse response (simplified)
        response.success = true;
        response.message = strdup(ai_response);
        response.suggestion = strdup("Try the suggested command");
        response.command = strdup(input); // Simplified
        free(ai_response);
    } else {
        response.message = strdup("Failed to get AI response");
    }

    return response;
}

// Get AI command type from input
AICommandType ai_get_command_type(const char *input) {
    if (!input) return AI_CMD_UNKNOWN;

    // Convert input to lowercase for comparison
    char *lower_input = strdup(input);
    for (char *p = lower_input; *p; p++) {
        *p = tolower(*p);
    }

    AICommandType type = AI_CMD_UNKNOWN;

    if (strstr(lower_input, "help") || strstr(lower_input, "how to")) {
        type = AI_CMD_HELP;
    } else if (strstr(lower_input, "explain") || strstr(lower_input, "what does")) {
        type = AI_CMD_EXPLAIN;
    } else if (strstr(lower_input, "suggest") || strstr(lower_input, "recommend")) {
        type = AI_CMD_SUGGEST;
    } else if (strstr(lower_input, "do") || strstr(lower_input, "execute")) {
        type = AI_CMD_EXECUTE;
    } else if (strstr(lower_input, "learn") || strstr(lower_input, "remember")) {
        type = AI_CMD_LEARN;
    }

    free(lower_input);
    return type;
}

// Generate command suggestion
char *ai_suggest_command(const char *description) {
    if (!ai_initialized) {
        return strdup("AI module not initialized");
    }

    char *prompt;
    asprintf(&prompt,
        "Suggest a shell command for: %s. "
        "Respond with just the command, no explanation.",
        description);

    char *response = make_api_request(prompt);
    free(prompt);
    return response;
}

// Explain a command
char *ai_explain_command(const char *command) {
    if (!ai_initialized) {
        return strdup("AI module not initialized");
    }

    char *prompt;
    asprintf(&prompt,
        "Explain what this shell command does: %s. "
        "Keep the explanation concise and clear.",
        command);

    char *response = make_api_request(prompt);
    free(prompt);
    return response;
}

// Learn from user feedback
void ai_learn(const char *input, const char *feedback) {
    (void)input; // Suppress unused parameter warning
    printf(COLOR_CYAN "Learning from feedback: %s\n" COLOR_RESET, feedback);
}

// Get AI status
bool ai_is_available(void) {
    return ai_initialized && ai_api_key != NULL;
} 