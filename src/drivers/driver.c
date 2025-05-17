#include "../../include/drivers/driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <dlfcn.h>

// Global variables
static Driver *drivers[MAX_DRIVERS];
static int driver_count = 0;
static pthread_mutex_t driver_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *driver_log = NULL;

// Function to write to driver log
static void write_driver_log(const char *format, ...) {
    if (!driver_log) return;

    time_t now;
    time(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    va_list args;
    va_start(args, format);
    fprintf(driver_log, "[%s] ", timestamp);
    vfprintf(driver_log, format, args);
    fprintf(driver_log, "\n");
    va_end(args);
}

// Initialize driver subsystem
int driver_init() {
    write_driver_log("Initializing driver subsystem...");

    // Initialize logging
    driver_log = fopen("logs/driver.log", "a");
    if (!driver_log) {
        perror("Failed to open driver log file");
        return -1;
    }
    setvbuf(driver_log, NULL, _IOLBF, 0);

    // Initialize driver array
    memset(drivers, 0, sizeof(drivers));
    driver_count = 0;

    write_driver_log("Driver subsystem initialized successfully");
    return 0;
}

// Cleanup driver subsystem
void driver_cleanup() {
    write_driver_log("Cleaning up driver subsystem...");

    // Unload all drivers
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i]) {
            driver_unregister(drivers[i]);
        }
    }

    // Close log file
    if (driver_log) {
        fclose(driver_log);
        driver_log = NULL;
    }

    write_driver_log("Driver subsystem cleanup completed");
}

// Register a new driver
Driver *driver_register(const char *name, DriverType type, DriverOps *ops, void *driver_data) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Registering driver: %s", name);

    // Check if maximum number of drivers reached
    if (driver_count >= MAX_DRIVERS) {
        write_driver_log("Maximum number of drivers reached");
        pthread_mutex_unlock(&driver_mutex);
        return NULL;
    }

    // Check if driver already exists
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] && strcmp(drivers[i]->name, name) == 0) {
            write_driver_log("Driver already exists: %s", name);
            pthread_mutex_unlock(&driver_mutex);
            return NULL;
        }
    }

    // Create new driver
    Driver *driver = (Driver *)malloc(sizeof(Driver));
    if (!driver) {
        write_driver_log("Failed to allocate memory for driver");
        pthread_mutex_unlock(&driver_mutex);
        return NULL;
    }

    // Initialize driver
    strncpy(driver->name, name, MAX_DRIVER_NAME_LENGTH - 1);
    driver->type = type;
    driver->state = DRIVER_STATE_UNINITIALIZED;
    memcpy(&driver->ops, ops, sizeof(DriverOps));
    driver->driver_data = driver_data;
    driver->major = 0;
    driver->minor = 0;
    driver->flags = 0;
    driver->features = 0;
    driver->capabilities = 0;
    driver->version = 0;
    driver->api_version = 0;
    memset(driver->description, 0, MAX_STRING_LENGTH);
    memset(driver->author, 0, MAX_STRING_LENGTH);
    memset(driver->license, 0, MAX_STRING_LENGTH);
    driver->load_time = time(NULL);
    driver->last_access = 0;
    driver->last_error = 0;
    driver->error_count = 0;
    memset(driver->error_message, 0, MAX_ERROR_MESSAGE_LENGTH);

    // Add to driver array
    drivers[driver_count++] = driver;

    write_driver_log("Driver registered successfully: %s", name);
    pthread_mutex_unlock(&driver_mutex);
    return driver;
}

// Unregister a driver
int driver_unregister(Driver *driver) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Unregistering driver: %s", driver->name);

    // Find driver in array
    int index = -1;
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] == driver) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        write_driver_log("Driver not found: %s", driver->name);
        pthread_mutex_unlock(&driver_mutex);
        return -1;
    }

    // Call cleanup operation if available
    if (driver->ops.cleanup) {
        driver->ops.cleanup(driver->driver_data);
    }

    // Remove from array
    for (int i = index; i < driver_count - 1; i++) {
        drivers[i] = drivers[i + 1];
    }
    driver_count--;

    // Free driver
    free(driver);

    write_driver_log("Driver unregistered successfully");
    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver by name
Driver *driver_get_by_name(const char *name) {
    pthread_mutex_lock(&driver_mutex);

    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] && strcmp(drivers[i]->name, name) == 0) {
            pthread_mutex_unlock(&driver_mutex);
            return drivers[i];
        }
    }

    pthread_mutex_unlock(&driver_mutex);
    return NULL;
}

// Get driver by type
Driver *driver_get_by_type(DriverType type) {
    pthread_mutex_lock(&driver_mutex);

    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] && drivers[i]->type == type) {
            pthread_mutex_unlock(&driver_mutex);
            return drivers[i];
        }
    }

    pthread_mutex_unlock(&driver_mutex);
    return NULL;
}

// List all drivers
int driver_list() {
    pthread_mutex_lock(&driver_mutex);

    printf("\nRegistered Drivers:\n");
    printf("------------------\n");
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i]) {
            printf("Name: %s\n", drivers[i]->name);
            printf("Type: %d\n", drivers[i]->type);
            printf("State: %d\n", drivers[i]->state);
            printf("Version: %u\n", drivers[i]->version);
            printf("API Version: %u\n", drivers[i]->api_version);
            printf("Description: %s\n", drivers[i]->description);
            printf("Author: %s\n", drivers[i]->author);
            printf("License: %s\n", drivers[i]->license);
            printf("Load Time: %s", ctime(&drivers[i]->load_time));
            printf("Last Access: %s", drivers[i]->last_access ? ctime(&drivers[i]->last_access) : "Never\n");
            printf("Last Error: %s", drivers[i]->last_error ? ctime(&drivers[i]->last_error) : "Never\n");
            printf("Error Count: %d\n", drivers[i]->error_count);
            if (drivers[i]->error_count > 0) {
                printf("Last Error Message: %s\n", drivers[i]->error_message);
            }
            printf("------------------\n");
        }
    }

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Load a driver from a shared library
int driver_load(const char *path) {
    write_driver_log("Loading driver from: %s", path);

    // Open shared library
    void *handle = dlopen(path, RTLD_NOW);
    if (!handle) {
        write_driver_log("Failed to load driver: %s", dlerror());
        return -1;
    }

    // Get driver initialization function
    typedef Driver *(*init_func_t)(void);
    init_func_t init_func = (init_func_t)dlsym(handle, "driver_init");
    if (!init_func) {
        write_driver_log("Failed to find driver initialization function: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    // Initialize driver
    Driver *driver = init_func();
    if (!driver) {
        write_driver_log("Driver initialization failed");
        dlclose(handle);
        return -1;
    }

    // Store handle in driver data
    driver->driver_data = handle;

    write_driver_log("Driver loaded successfully: %s", driver->name);
    return 0;
}

// Unload a driver
int driver_unload(const char *name) {
    write_driver_log("Unloading driver: %s", name);

    Driver *driver = driver_get_by_name(name);
    if (!driver) {
        write_driver_log("Driver not found: %s", name);
        return -1;
    }

    // Get driver cleanup function
    void *handle = driver->driver_data;
    typedef void (*cleanup_func_t)(Driver *);
    cleanup_func_t cleanup_func = (cleanup_func_t)dlsym(handle, "driver_cleanup");
    if (cleanup_func) {
        cleanup_func(driver);
    }

    // Unregister driver
    int result = driver_unregister(driver);
    if (result != 0) {
        write_driver_log("Failed to unregister driver: %s", name);
        return -1;
    }

    // Close shared library
    dlclose(handle);

    write_driver_log("Driver unloaded successfully: %s", name);
    return 0;
}

// Suspend a driver
int driver_suspend(Driver *driver) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Suspending driver: %s", driver->name);

    if (driver->state != DRIVER_STATE_RUNNING) {
        write_driver_log("Driver is not running: %s", driver->name);
        pthread_mutex_unlock(&driver_mutex);
        return -1;
    }

    driver->state = DRIVER_STATE_SUSPENDED;

    write_driver_log("Driver suspended successfully: %s", driver->name);
    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Resume a driver
int driver_resume(Driver *driver) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Resuming driver: %s", driver->name);

    if (driver->state != DRIVER_STATE_SUSPENDED) {
        write_driver_log("Driver is not suspended: %s", driver->name);
        pthread_mutex_unlock(&driver_mutex);
        return -1;
    }

    driver->state = DRIVER_STATE_RUNNING;

    write_driver_log("Driver resumed successfully: %s", driver->name);
    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Reset a driver
int driver_reset(Driver *driver) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Resetting driver: %s", driver->name);

    // Call cleanup operation if available
    if (driver->ops.cleanup) {
        driver->ops.cleanup(driver->driver_data);
    }

    // Call init operation if available
    if (driver->ops.init) {
        int result = driver->ops.init(driver->driver_data);
        if (result != 0) {
            write_driver_log("Driver reset failed: %s", driver->name);
            pthread_mutex_unlock(&driver_mutex);
            return -1;
        }
    }

    driver->state = DRIVER_STATE_INITIALIZED;
    driver->error_count = 0;
    memset(driver->error_message, 0, MAX_ERROR_MESSAGE_LENGTH);

    write_driver_log("Driver reset successfully: %s", driver->name);
    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Set driver state
int driver_set_state(Driver *driver, DriverState state) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver state: %s -> %d", driver->name, state);

    driver->state = state;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver state
DriverState driver_get_state(Driver *driver) {
    return driver->state;
}

// Set driver error
int driver_set_error(Driver *driver, const char *error_message) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver error: %s -> %s", driver->name, error_message);

    strncpy(driver->error_message, error_message, MAX_ERROR_MESSAGE_LENGTH - 1);
    driver->last_error = time(NULL);
    driver->error_count++;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver error
const char *driver_get_error(Driver *driver) {
    return driver->error_message;
}

// Clear driver error
int driver_clear_error(Driver *driver) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Clearing driver error: %s", driver->name);

    memset(driver->error_message, 0, MAX_ERROR_MESSAGE_LENGTH);
    driver->last_error = 0;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Set driver feature
int driver_set_feature(Driver *driver, uint32_t feature) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver feature: %s -> %u", driver->name, feature);

    driver->features |= feature;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Clear driver feature
int driver_clear_feature(Driver *driver, uint32_t feature) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Clearing driver feature: %s -> %u", driver->name, feature);

    driver->features &= ~feature;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Check if driver has feature
int driver_has_feature(Driver *driver, uint32_t feature) {
    return (driver->features & feature) != 0;
}

// Set driver capability
int driver_set_capability(Driver *driver, uint32_t capability) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver capability: %s -> %u", driver->name, capability);

    driver->capabilities |= capability;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Clear driver capability
int driver_clear_capability(Driver *driver, uint32_t capability) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Clearing driver capability: %s -> %u", driver->name, capability);

    driver->capabilities &= ~capability;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Check if driver has capability
int driver_has_capability(Driver *driver, uint32_t capability) {
    return (driver->capabilities & capability) != 0;
}

// Set driver version
int driver_set_version(Driver *driver, uint32_t version) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver version: %s -> %u", driver->name, version);

    driver->version = version;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver version
uint32_t driver_get_version(Driver *driver) {
    return driver->version;
}

// Set driver API version
int driver_set_api_version(Driver *driver, uint32_t api_version) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver API version: %s -> %u", driver->name, api_version);

    driver->api_version = api_version;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver API version
uint32_t driver_get_api_version(Driver *driver) {
    return driver->api_version;
}

// Set driver description
int driver_set_description(Driver *driver, const char *description) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver description: %s -> %s", driver->name, description);

    strncpy(driver->description, description, MAX_STRING_LENGTH - 1);

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver description
const char *driver_get_description(Driver *driver) {
    return driver->description;
}

// Set driver author
int driver_set_author(Driver *driver, const char *author) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver author: %s -> %s", driver->name, author);

    strncpy(driver->author, author, MAX_STRING_LENGTH - 1);

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver author
const char *driver_get_author(Driver *driver) {
    return driver->author;
}

// Set driver license
int driver_set_license(Driver *driver, const char *license) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver license: %s -> %s", driver->name, license);

    strncpy(driver->license, license, MAX_STRING_LENGTH - 1);

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver license
const char *driver_get_license(Driver *driver) {
    return driver->license;
}

// Get driver load time
time_t driver_get_load_time(Driver *driver) {
    return driver->load_time;
}

// Get driver last access time
time_t driver_get_last_access(Driver *driver) {
    return driver->last_access;
}

// Get driver last error time
time_t driver_get_last_error(Driver *driver) {
    return driver->last_error;
}

// Get driver error count
int driver_get_error_count(Driver *driver) {
    return driver->error_count;
}

// Increment driver error count
int driver_increment_error_count(Driver *driver) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Incrementing driver error count: %s", driver->name);

    driver->error_count++;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Reset driver error count
int driver_reset_error_count(Driver *driver) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Resetting driver error count: %s", driver->name);

    driver->error_count = 0;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Set driver major number
int driver_set_major(Driver *driver, int major) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver major number: %s -> %d", driver->name, major);

    driver->major = major;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver major number
int driver_get_major(Driver *driver) {
    return driver->major;
}

// Set driver minor number
int driver_set_minor(Driver *driver, int minor) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver minor number: %s -> %d", driver->name, minor);

    driver->minor = minor;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver minor number
int driver_get_minor(Driver *driver) {
    return driver->minor;
}

// Set driver flags
int driver_set_flags(Driver *driver, int flags) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver flags: %s -> %d", driver->name, flags);

    driver->flags = flags;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver flags
int driver_get_flags(Driver *driver) {
    return driver->flags;
}

// Set driver data
int driver_set_driver_data(Driver *driver, void *driver_data) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver data: %s", driver->name);

    driver->driver_data = driver_data;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver data
void *driver_get_driver_data(Driver *driver) {
    return driver->driver_data;
}

// Set driver operations
int driver_set_ops(Driver *driver, DriverOps *ops) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver operations: %s", driver->name);

    memcpy(&driver->ops, ops, sizeof(DriverOps));

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver operations
DriverOps *driver_get_ops(Driver *driver) {
    return &driver->ops;
}

// Set driver type
int driver_set_type(Driver *driver, DriverType type) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver type: %s -> %d", driver->name, type);

    driver->type = type;

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver type
DriverType driver_get_type(Driver *driver) {
    return driver->type;
}

// Set driver name
int driver_set_name(Driver *driver, const char *name) {
    pthread_mutex_lock(&driver_mutex);

    write_driver_log("Setting driver name: %s -> %s", driver->name, name);

    strncpy(driver->name, name, MAX_DRIVER_NAME_LENGTH - 1);

    pthread_mutex_unlock(&driver_mutex);
    return 0;
}

// Get driver name
const char *driver_get_name(Driver *driver) {
    return driver->name;
} 