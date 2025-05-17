#include "../../include/drivers/driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global driver registry
static Driver *driver_registry[MAX_DRIVERS] = {NULL};
static int driver_count = 0;

// Initialize the driver subsystem
int driver_init(void) {
    // Clear the driver registry
    memset(driver_registry, 0, sizeof(driver_registry));
    driver_count = 0;
    return 0;
}

// Cleanup the driver subsystem
void driver_cleanup(void) {
    // Unregister all drivers
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (driver_registry[i]) {
            driver_unregister(driver_registry[i]);
        }
    }
    driver_count = 0;
}

// Register a new driver
Driver *driver_register(const char *name, DriverType type, DriverOps *ops, void *driver_data) {
    if (driver_count >= MAX_DRIVERS) {
        return NULL;
    }

    // Create new driver
    Driver *driver = (Driver *)malloc(sizeof(Driver));
    if (!driver) {
        return NULL;
    }

    // Initialize driver
    strncpy(driver->name, name, sizeof(driver->name) - 1);
    driver->name[sizeof(driver->name) - 1] = '\0';
    driver->type = type;
    memcpy(&driver->ops, ops, sizeof(DriverOps));
    driver->driver_data = driver_data;
    driver->state = DRIVER_STATE_UNINITIALIZED;
    driver->version = 0;
    driver->api_version = 0;
    memset(driver->description, 0, sizeof(driver->description));
    memset(driver->author, 0, sizeof(driver->author));
    memset(driver->license, 0, sizeof(driver->license));

    // Add to registry
    driver_registry[driver_count++] = driver;
    driver->state = DRIVER_STATE_INITIALIZED;

    return driver;
}

// Unregister a driver
int driver_unregister(Driver *driver) {
    if (!driver) {
        return -1;
    }

    // Find driver in registry
    for (int i = 0; i < driver_count; i++) {
        if (driver_registry[i] == driver) {
            // Cleanup driver
            if (driver->ops.cleanup) {
                driver->ops.cleanup(driver->driver_data);
            }

            // Remove from registry
            driver_registry[i] = driver_registry[--driver_count];
            driver_registry[driver_count] = NULL;
            free(driver);
            return 0;
        }
    }
    return -1;
}

// Get driver by name
Driver *driver_get_by_name(const char *name) {
    for (int i = 0; i < driver_count; i++) {
        if (driver_registry[i] && strcmp(driver_registry[i]->name, name) == 0) {
            return driver_registry[i];
        }
    }
    return NULL;
}

// Get driver operations
DriverOps *driver_get_ops(Driver *driver) {
    return driver ? &driver->ops : NULL;
}

// Get driver data
void *driver_get_driver_data(Driver *driver) {
    return driver ? driver->driver_data : NULL;
}

// Get driver name
const char *driver_get_name(Driver *driver) {
    return driver ? driver->name : NULL;
}

// Get driver type
DriverType driver_get_type(Driver *driver) {
    return driver ? driver->type : 0;
}

// Get driver version
uint32_t driver_get_version(Driver *driver) {
    return driver ? driver->version : 0;
}

// Get driver API version
uint32_t driver_get_api_version(Driver *driver) {
    return driver ? driver->api_version : 0;
}

// Get driver description
const char *driver_get_description(Driver *driver) {
    return driver ? driver->description : NULL;
}

// Get driver author
const char *driver_get_author(Driver *driver) {
    return driver ? driver->author : NULL;
}

// Get driver license
const char *driver_get_license(Driver *driver) {
    return driver ? driver->license : NULL;
}

// Set driver version
int driver_set_version(Driver *driver, uint32_t version) {
    if (driver) {
        driver->version = version;
        return 0;
    }
    return -1;
}

// Set driver API version
int driver_set_api_version(Driver *driver, uint32_t api_version) {
    if (driver) {
        driver->api_version = api_version;
        return 0;
    }
    return -1;
}

// Set driver description
int driver_set_description(Driver *driver, const char *description) {
    if (driver) {
        strncpy(driver->description, description ? description : "", sizeof(driver->description) - 1);
        driver->description[sizeof(driver->description) - 1] = '\0';
        return 0;
    }
    return -1;
}

// Set driver author
int driver_set_author(Driver *driver, const char *author) {
    if (driver) {
        strncpy(driver->author, author ? author : "", sizeof(driver->author) - 1);
        driver->author[sizeof(driver->author) - 1] = '\0';
        return 0;
    }
    return -1;
}

// Set driver license
int driver_set_license(Driver *driver, const char *license) {
    if (driver) {
        strncpy(driver->license, license ? license : "", sizeof(driver->license) - 1);
        driver->license[sizeof(driver->license) - 1] = '\0';
        return 0;
    }
    return -1;
} 