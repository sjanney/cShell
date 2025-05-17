#ifndef DRIVER_H
#define DRIVER_H

#include "../kernel/kernel.h"
#include <stdint.h>

// Driver types
typedef enum {
    DRIVER_TYPE_CHAR,
    DRIVER_TYPE_BLOCK,
    DRIVER_TYPE_NETWORK,
    DRIVER_TYPE_INPUT,
    DRIVER_TYPE_DISPLAY,
    DRIVER_TYPE_SOUND,
    DRIVER_TYPE_STORAGE,
    DRIVER_TYPE_PRINTER,
    DRIVER_TYPE_SCANNER,
    DRIVER_TYPE_CAMERA,
    DRIVER_TYPE_SENSOR,
    DRIVER_TYPE_OTHER
} DriverType;

// Driver states
typedef enum {
    DRIVER_STATE_UNINITIALIZED,
    DRIVER_STATE_INITIALIZED,
    DRIVER_STATE_RUNNING,
    DRIVER_STATE_SUSPENDED,
    DRIVER_STATE_ERROR
} DriverState;

// Driver operations
typedef struct {
    int (*init)(void *driver_data);
    int (*cleanup)(void *driver_data);
    int (*open)(void *driver_data, int flags);
    int (*close)(void *driver_data);
    ssize_t (*read)(void *driver_data, void *buffer, size_t size);
    ssize_t (*write)(void *driver_data, const void *buffer, size_t size);
    int (*ioctl)(void *driver_data, unsigned long request, void *arg);
    int (*poll)(void *driver_data, int events);
    int (*mmap)(void *driver_data, void *addr, size_t length, int prot, int flags, off_t offset);
    int (*munmap)(void *driver_data, void *addr, size_t length);
} DriverOps;

// Driver structure
typedef struct {
    char name[MAX_DRIVER_NAME_LENGTH];
    DriverType type;
    DriverState state;
    DriverOps ops;
    void *driver_data;
    int major;
    int minor;
    int flags;
    uint32_t features;
    uint32_t capabilities;
    uint32_t version;
    uint32_t api_version;
    char description[MAX_STRING_LENGTH];
    char author[MAX_STRING_LENGTH];
    char license[MAX_STRING_LENGTH];
    time_t load_time;
    time_t last_access;
    time_t last_error;
    int error_count;
    char error_message[MAX_ERROR_MESSAGE_LENGTH];
} Driver;

// Driver management functions
int driver_init();
void driver_cleanup();
Driver *driver_register(const char *name, DriverType type, DriverOps *ops, void *driver_data);
int driver_unregister(Driver *driver);
Driver *driver_get_by_name(const char *name);
Driver *driver_get_by_type(DriverType type);
int driver_list();
int driver_load(const char *path);
int driver_unload(const char *name);
int driver_suspend(Driver *driver);
int driver_resume(Driver *driver);
int driver_reset(Driver *driver);
int driver_set_state(Driver *driver, DriverState state);
DriverState driver_get_state(Driver *driver);
int driver_set_error(Driver *driver, const char *error_message);
const char *driver_get_error(Driver *driver);
int driver_clear_error(Driver *driver);
int driver_set_feature(Driver *driver, uint32_t feature);
int driver_clear_feature(Driver *driver, uint32_t feature);
int driver_has_feature(Driver *driver, uint32_t feature);
int driver_set_capability(Driver *driver, uint32_t capability);
int driver_clear_capability(Driver *driver, uint32_t capability);
int driver_has_capability(Driver *driver, uint32_t capability);
int driver_set_version(Driver *driver, uint32_t version);
uint32_t driver_get_version(Driver *driver);
int driver_set_api_version(Driver *driver, uint32_t api_version);
uint32_t driver_get_api_version(Driver *driver);
int driver_set_description(Driver *driver, const char *description);
const char *driver_get_description(Driver *driver);
int driver_set_author(Driver *driver, const char *author);
const char *driver_get_author(Driver *driver);
int driver_set_license(Driver *driver, const char *license);
const char *driver_get_license(Driver *driver);
time_t driver_get_load_time(Driver *driver);
time_t driver_get_last_access(Driver *driver);
time_t driver_get_last_error(Driver *driver);
int driver_get_error_count(Driver *driver);
int driver_increment_error_count(Driver *driver);
int driver_reset_error_count(Driver *driver);
int driver_set_major(Driver *driver, int major);
int driver_get_major(Driver *driver);
int driver_set_minor(Driver *driver, int minor);
int driver_get_minor(Driver *driver);
int driver_set_flags(Driver *driver, int flags);
int driver_get_flags(Driver *driver);
int driver_set_driver_data(Driver *driver, void *driver_data);
void *driver_get_driver_data(Driver *driver);
int driver_set_ops(Driver *driver, DriverOps *ops);
DriverOps *driver_get_ops(Driver *driver);
int driver_set_type(Driver *driver, DriverType type);
DriverType driver_get_type(Driver *driver);
int driver_set_name(Driver *driver, const char *name);
const char *driver_get_name(Driver *driver);

#endif // DRIVER_H 