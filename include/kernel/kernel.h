#ifndef CSHELL_KERNEL_H
#define CSHELL_KERNEL_H

#include <stdbool.h>
#include <stdint.h>

// Kernel version
#define KERNEL_VERSION "1.0.0"

// Kernel configuration
#define KERNEL_MAX_PROCESSES 1024
#define KERNEL_MAX_THREADS 4096
#define KERNEL_MAX_DEVICES 256
#define KERNEL_MAX_DRIVERS 128
#define KERNEL_MAX_MEMORY 1024 * 1024 * 1024  // 1GB
#define KERNEL_STACK_SIZE 8192
#define KERNEL_HEAP_SIZE 1024 * 1024  // 1MB

// Kernel error codes
typedef enum {
    KERNEL_SUCCESS = 0,
    KERNEL_ERROR = -1,
    KERNEL_ERROR_INVALID_ARGUMENT = -2,
    KERNEL_ERROR_OUT_OF_MEMORY = -3,
    KERNEL_ERROR_NOT_FOUND = -4,
    KERNEL_ERROR_ALREADY_EXISTS = -5,
    KERNEL_ERROR_PERMISSION_DENIED = -6,
    KERNEL_ERROR_TIMEOUT = -7,
    KERNEL_ERROR_BUSY = -8,
    KERNEL_ERROR_IO = -9,
    KERNEL_ERROR_INTERRUPTED = -10
} KernelError;

// Kernel status
typedef enum {
    KERNEL_STATUS_UNINITIALIZED,
    KERNEL_STATUS_INITIALIZING,
    KERNEL_STATUS_RUNNING,
    KERNEL_STATUS_SHUTTING_DOWN,
    KERNEL_STATUS_SHUTDOWN
} KernelStatus;

// Kernel configuration structure
typedef struct {
    char *name;
    char *version;
    uint32_t max_processes;
    uint32_t max_threads;
    uint32_t max_devices;
    uint32_t max_drivers;
    uint64_t max_memory;
    uint32_t stack_size;
    uint32_t heap_size;
    bool debug_mode;
    bool verbose_mode;
    char *log_file;
} KernelConfig;

// Kernel statistics
typedef struct {
    uint32_t process_count;
    uint32_t thread_count;
    uint32_t device_count;
    uint32_t driver_count;
    uint64_t memory_used;
    uint64_t memory_free;
    uint32_t uptime;
    uint32_t load_average[3];
} KernelStats;

// Initialize kernel
int kernel_init(void);

// Clean up kernel
void kernel_cleanup(void);

// Get kernel version
const char *kernel_get_version(void);

// Get kernel status
KernelStatus kernel_get_status(void);

// Get kernel statistics
KernelStats kernel_get_stats(void);

// Set kernel configuration
int kernel_set_config(const KernelConfig *config);

// Get kernel configuration
const KernelConfig *kernel_get_config(void);

// Log kernel message
void kernel_log(const char *format, ...);

// Get kernel error message
const char *kernel_strerror(KernelError error);

#endif // CSHELL_KERNEL_H 