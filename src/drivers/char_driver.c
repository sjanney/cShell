#include "../../include/drivers/driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define CHAR_DRIVER_BUFFER_SIZE 1024

typedef struct {
    char buffer[CHAR_DRIVER_BUFFER_SIZE];
    size_t write_pos;
    size_t read_pos;
    int is_open;
} CharDriverData;

// Forward declarations of driver operations
static int char_driver_init(void *driver_data);
static int char_driver_cleanup(void *driver_data);
static int char_driver_open(void *driver_data, int flags);
static int char_driver_close(void *driver_data);
static ssize_t char_driver_read(void *driver_data, void *buffer, size_t size);
static ssize_t char_driver_write(void *driver_data, const void *buffer, size_t size);
static int char_driver_ioctl(void *driver_data, unsigned long request, void *arg);

// Driver operations structure
static DriverOps char_driver_ops = {
    .init = char_driver_init,
    .cleanup = char_driver_cleanup,
    .open = char_driver_open,
    .close = char_driver_close,
    .read = char_driver_read,
    .write = char_driver_write,
    .ioctl = char_driver_ioctl,
    .poll = NULL,
    .mmap = NULL,
    .munmap = NULL
};

// Initialize the character driver
static int char_driver_init(void *driver_data) {
    CharDriverData *data = (CharDriverData *)driver_data;
    
    // Initialize buffer
    memset(data->buffer, 0, CHAR_DRIVER_BUFFER_SIZE);
    data->write_pos = 0;
    data->read_pos = 0;
    data->is_open = 0;
    
    return 0;
}

// Cleanup the character driver
static int char_driver_cleanup(void *driver_data) {
    CharDriverData *data = (CharDriverData *)driver_data;
    
    // Clear buffer
    memset(data->buffer, 0, CHAR_DRIVER_BUFFER_SIZE);
    data->write_pos = 0;
    data->read_pos = 0;
    data->is_open = 0;
    
    return 0;
}

// Open the character driver
static int char_driver_open(void *driver_data, int flags) {
    CharDriverData *data = (CharDriverData *)driver_data;
    
    if (data->is_open) {
        return -EBUSY;
    }
    
    // Check if flags are valid
    if (flags & ~(O_RDONLY | O_WRONLY | O_RDWR)) {
        return -EINVAL;
    }
    
    data->is_open = 1;
    return 0;
}

// Close the character driver
static int char_driver_close(void *driver_data) {
    CharDriverData *data = (CharDriverData *)driver_data;
    
    if (!data->is_open) {
        return -ENODEV;
    }
    
    data->is_open = 0;
    return 0;
}

// Read from the character driver
static ssize_t char_driver_read(void *driver_data, void *buffer, size_t size) {
    CharDriverData *data = (CharDriverData *)driver_data;
    
    if (!data->is_open) {
        return -ENODEV;
    }
    
    if (data->read_pos >= data->write_pos) {
        return 0;  // No data available
    }
    
    size_t available = data->write_pos - data->read_pos;
    size_t to_read = (size < available) ? size : available;
    
    memcpy(buffer, &data->buffer[data->read_pos], to_read);
    data->read_pos += to_read;
    
    return to_read;
}

// Write to the character driver
static ssize_t char_driver_write(void *driver_data, const void *buffer, size_t size) {
    CharDriverData *data = (CharDriverData *)driver_data;
    
    if (!data->is_open) {
        return -ENODEV;
    }
    
    if (data->write_pos >= CHAR_DRIVER_BUFFER_SIZE) {
        return -ENOSPC;  // Buffer full
    }
    
    size_t available = CHAR_DRIVER_BUFFER_SIZE - data->write_pos;
    size_t to_write = (size < available) ? size : available;
    
    memcpy(&data->buffer[data->write_pos], buffer, to_write);
    data->write_pos += to_write;
    
    return to_write;
}

// IOCTL operations for the character driver
static int char_driver_ioctl(void *driver_data, unsigned long request, void *arg) {
    CharDriverData *data = (CharDriverData *)driver_data;
    
    if (!data->is_open) {
        return -ENODEV;
    }
    
    switch (request) {
        case 0x1:  // Clear buffer
            data->write_pos = 0;
            data->read_pos = 0;
            return 0;
            
        case 0x2:  // Get buffer size
            if (arg) {
                *(size_t *)arg = CHAR_DRIVER_BUFFER_SIZE;
                return 0;
            }
            return -EINVAL;
            
        case 0x3:  // Get available space
            if (arg) {
                *(size_t *)arg = CHAR_DRIVER_BUFFER_SIZE - data->write_pos;
                return 0;
            }
            return -EINVAL;
            
        default:
            return -ENOTTY;
    }
}

// Driver initialization function
Driver *char_driver_create(void) {
    // Allocate driver data
    CharDriverData *driver_data = (CharDriverData *)malloc(sizeof(CharDriverData));
    if (!driver_data) {
        return NULL;
    }
    
    // Initialize driver data
    if (char_driver_init(driver_data) != 0) {
        free(driver_data);
        return NULL;
    }
    
    // Create and register driver
    Driver *driver = driver_register("char_driver", DRIVER_TYPE_CHAR, &char_driver_ops, driver_data);
    if (!driver) {
        char_driver_cleanup(driver_data);
        free(driver_data);
        return NULL;
    }
    
    // Set driver properties
    driver_set_version(driver, 0x00010000);  // Version 1.0.0
    driver_set_api_version(driver, 0x00010000);  // API Version 1.0.0
    driver_set_description(driver, "Simple character device driver");
    driver_set_author(driver, "cShell Team");
    driver_set_license(driver, "MIT");
    
    return driver;
}

// Driver cleanup function
static void char_driver_destroy(Driver *driver) {
    if (!driver) {
        return;
    }
    
    CharDriverData *driver_data = (CharDriverData *)driver_get_driver_data(driver);
    if (driver_data) {
        char_driver_cleanup(driver_data);
        free(driver_data);
    }
} 