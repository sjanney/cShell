#include "../include/kernel/kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>     // For va_list, va_start, va_end
#include <pwd.h>        // For passwd structure and getpwuid
#include <sys/mman.h>   // For mmap, munmap

// Global variables
static SystemInfo system_info;
static Process *processes[MAX_PROCESSES] = {NULL};
static Thread *threads[MAX_THREADS] = {NULL};
static Device *devices[MAX_DEVICES] = {NULL};
static File *files[MAX_FILES] = {NULL};
static MemoryBlock *memory_blocks[MAX_MEMORY_BLOCKS] = {NULL};
static IpcMessage *ipc_messages[MAX_IPC_QUEUES] = {NULL};
static Syscall *syscalls[MAX_SYSCALLS] = {NULL};
static Error *errors[MAX_ERROR_CODES] = {NULL};
static LogEntry *log_entries[MAX_LOG_ENTRIES] = {NULL};

// Mutexes for thread safety
static pthread_mutex_t process_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t device_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ipc_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t syscall_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Log file
static FILE *log_file = NULL;

// Function to initialize logging
static void init_logging() {
    // Make sure logs directory exists
    mkdir("logs", 0755);
    
    log_file = fopen("logs/kernel.log", "a");
    if (!log_file) {
        perror("Failed to open log file");
        exit(1);
    }
    setvbuf(log_file, NULL, _IOLBF, 0);
}

// Function to write to log
static void write_log(LogLevel level, const char *format, ...) {
    if (!log_file) {
        init_logging();
    }

    time_t now;
    time(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    const char *level_str;
    switch (level) {
        case LOG_LEVEL_DEBUG: level_str = "DEBUG"; break;
        case LOG_LEVEL_INFO: level_str = "INFO"; break;
        case LOG_LEVEL_WARNING: level_str = "WARNING"; break;
        case LOG_LEVEL_ERROR: level_str = "ERROR"; break;
        case LOG_LEVEL_CRITICAL: level_str = "CRITICAL"; break;
        default: level_str = "UNKNOWN";
    }

    va_list args;
    va_start(args, format);
    fprintf(log_file, "[%s] [%s] ", timestamp, level_str);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    va_end(args);
}

// Initialize the kernel
int kernel_init(void) {
    // Initialize logging
    init_logging();
    write_log(LOG_LEVEL_INFO, "Initializing kernel...");
    
    // Initialize system info
    memset(&system_info, 0, sizeof(SystemInfo));
    
    // Set basic system info
    gethostname(system_info.hostname, sizeof(system_info.hostname));
    strncpy(system_info.os_name, "cShell", sizeof(system_info.os_name));
    strncpy(system_info.kernel_version, "1.0.0", sizeof(system_info.kernel_version));
    system_info.state = SYSTEM_STATE_INIT;
    system_info.boot_time = time(NULL);
    system_info.uptime = 0;
    
    // Initialize memory info
    system_info.total_memory = 1024 * 1024 * 1024;  // 1GB default
    system_info.free_memory = system_info.total_memory;
    system_info.used_memory = 0;
    
    // Initialize counters
    system_info.cpu_count = 1;  // Default to 1 CPU
    system_info.process_count = 0;
    system_info.thread_count = 0;
    system_info.device_count = 0;
    system_info.file_count = 0;
    system_info.memory_block_count = 0;
    system_info.ipc_queue_count = 0;
    system_info.semaphore_count = 0;
    system_info.mutex_count = 0;
    system_info.event_count = 0;
    system_info.timer_count = 0;
    system_info.irq_count = 0;
    system_info.driver_count = 0;
    system_info.module_count = 0;
    system_info.system_call_count = 0;
    system_info.error_count = 0;
    system_info.log_count = 0;
    
    // Set system state to running
    system_info.state = SYSTEM_STATE_RUNNING;
    
    write_log(LOG_LEVEL_INFO, "Kernel initialized successfully");
    return 0;
}

// Kernel cleanup
void kernel_cleanup() {
    write_log(LOG_LEVEL_INFO, "Cleaning up kernel...");

    // Set system state
    system_info.state = SYSTEM_STATE_HALTED;

    // Close log file
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }

    write_log(LOG_LEVEL_INFO, "Kernel cleanup complete");
}

// Kernel panic
void kernel_panic(const char *message) {
    write_log(LOG_LEVEL_CRITICAL, "KERNEL PANIC: %s", message);
    system_info.state = SYSTEM_STATE_ERROR;
    exit(1);
}

// Process management
Process *process_create(const char *name, int priority) {
    pthread_mutex_lock(&process_mutex);

    if (system_info.process_count >= MAX_PROCESSES) {
        pthread_mutex_unlock(&process_mutex);
        write_log(LOG_LEVEL_ERROR, "Maximum number of processes reached");
        return NULL;
    }

    Process *process = (Process *)malloc(sizeof(Process));
    if (!process) {
        pthread_mutex_unlock(&process_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate memory for process");
        return NULL;
    }

    // Initialize process
    process->pid = getpid();
    strncpy(process->name, name, MAX_STRING_LENGTH - 1);
    process->state = PROCESS_STATE_CREATED;
    process->priority = priority;
    process->memory_usage = 0;
    process->start_time = time(NULL);
    process->parent_pid = getppid();
    process->exit_code = 0;

    // Get owner information
    struct passwd *pwd = getpwuid(getuid());
    if (pwd) {
        strncpy(process->owner, pwd->pw_name, MAX_USERNAME_LENGTH - 1);
    } else {
        strncpy(process->owner, "unknown", MAX_USERNAME_LENGTH - 1);
    }

    // Allocate memory for process
    process->stack = malloc(MAX_STACK_SIZE);
    if (!process->stack) {
        free(process);
        pthread_mutex_unlock(&process_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate stack for process");
        return NULL;
    }

    process->heap = malloc(MAX_HEAP_SIZE);
    if (!process->heap) {
        free(process->stack);
        free(process);
        pthread_mutex_unlock(&process_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate heap for process");
        return NULL;
    }

    // Add process to list
    processes[system_info.process_count++] = process;

    pthread_mutex_unlock(&process_mutex);
    write_log(LOG_LEVEL_INFO, "Created process %s (PID: %d)", name, process->pid);
    return process;
}

// Thread management
Thread *thread_create(Process *process, const char *name, int priority) {
    pthread_mutex_lock(&thread_mutex);

    if (system_info.thread_count >= MAX_THREADS) {
        pthread_mutex_unlock(&thread_mutex);
        write_log(LOG_LEVEL_ERROR, "Maximum number of threads reached");
        return NULL;
    }

    Thread *thread = (Thread *)malloc(sizeof(Thread));
    if (!thread) {
        pthread_mutex_unlock(&thread_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate memory for thread");
        return NULL;
    }

    // Initialize thread
    strncpy(thread->name, name, MAX_STRING_LENGTH - 1);
    thread->state = THREAD_STATE_CREATED;
    thread->priority = priority;
    thread->process = process;

    // Allocate stack for thread
    thread->stack = malloc(MAX_STACK_SIZE);
    if (!thread->stack) {
        free(thread);
        pthread_mutex_unlock(&thread_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate stack for thread");
        return NULL;
    }

    // Add thread to list
    threads[system_info.thread_count++] = thread;

    pthread_mutex_unlock(&thread_mutex);
    write_log(LOG_LEVEL_INFO, "Created thread %s in process %s", name, process->name);
    return thread;
}

// Memory management
MemoryBlock *memory_allocate(Process *process, size_t size) {
    pthread_mutex_lock(&memory_mutex);

    if (system_info.memory_block_count >= MAX_MEMORY_BLOCKS) {
        pthread_mutex_unlock(&memory_mutex);
        write_log(LOG_LEVEL_ERROR, "Maximum number of memory blocks reached");
        return NULL;
    }

    MemoryBlock *block = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    if (!block) {
        pthread_mutex_unlock(&memory_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate memory for block structure");
        return NULL;
    }

    // Initialize memory block
    block->id = system_info.memory_block_count;
    snprintf(block->name, MAX_STRING_LENGTH, "block_%d", block->id);
    block->size = size;
    block->state = MEMORY_STATE_ALLOCATED;
    block->owner = process;
    block->allocation_time = time(NULL);
    block->deallocation_time = 0;

    // Allocate memory
    block->address = malloc(size);
    if (!block->address) {
        free(block);
        pthread_mutex_unlock(&memory_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate memory block");
        return NULL;
    }

    // Add block to list
    memory_blocks[system_info.memory_block_count++] = block;

    pthread_mutex_unlock(&memory_mutex);
    write_log(LOG_LEVEL_INFO, "Allocated %zu bytes for process %s", size, process->name);
    return block;
}

// Device management
Device *device_register(const char *name, const char *type) {
    pthread_mutex_lock(&device_mutex);

    if (system_info.device_count >= MAX_DEVICES) {
        pthread_mutex_unlock(&device_mutex);
        write_log(LOG_LEVEL_ERROR, "Maximum number of devices reached");
        return NULL;
    }

    Device *device = (Device *)malloc(sizeof(Device));
    if (!device) {
        pthread_mutex_unlock(&device_mutex);
        write_log(LOG_LEVEL_ERROR, "Failed to allocate memory for device");
        return NULL;
    }

    // Initialize device
    device->id = system_info.device_count;
    strncpy(device->name, name, MAX_STRING_LENGTH - 1);
    strncpy(device->type, type, MAX_STRING_LENGTH - 1);
    device->state = DEVICE_STATE_READY;
    device->last_access = time(NULL);
    device->last_error = 0;
    device->error_count = 0;
    memset(device->error_message, 0, MAX_ERROR_MESSAGE_LENGTH);

    // Add device to list
    devices[system_info.device_count++] = device;

    pthread_mutex_unlock(&device_mutex);
    write_log(LOG_LEVEL_INFO, "Registered device %s of type %s", name, type);
    return device;
}

// System information
SystemInfo *system_info_get() {
    return &system_info;
}

void system_info_update() {
    // Update system uptime
    system_info.uptime = time(NULL) - system_info.boot_time;
    
    // On macOS, we don't have sysinfo, so we use fixed values for memory
    // In a real implementation, you would use platform-specific APIs to get memory info
}

void system_info_print() {
    printf("\n=== System Information ===\n");
    printf("Hostname: %s\n", system_info.hostname);
    printf("OS: %s\n", system_info.os_name);
    printf("Kernel Version: %s\n", system_info.kernel_version);
    printf("System State: %d\n", system_info.state);
    printf("Boot Time: %s", ctime(&system_info.boot_time));
    printf("Uptime: %ld seconds\n", system_info.uptime);
    printf("Total Memory: %zu bytes\n", system_info.total_memory);
    printf("Free Memory: %zu bytes\n", system_info.free_memory);
    printf("Used Memory: %zu bytes\n", system_info.used_memory);
    printf("CPU Count: %d\n", system_info.cpu_count);
    printf("Process Count: %d\n", system_info.process_count);
    printf("Thread Count: %d\n", system_info.thread_count);
    printf("Device Count: %d\n", system_info.device_count);
    printf("File Count: %d\n", system_info.file_count);
    printf("Memory Block Count: %d\n", system_info.memory_block_count);
    printf("=======================\n\n");
} 