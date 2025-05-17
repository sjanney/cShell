#include "../../include/drivers/driver.h"
#include "../../include/drivers/char_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TEST_BUFFER_SIZE 256

// Forward declaration of driver creation function
extern Driver *char_driver_create(void);

int main() {
    // Initialize driver subsystem
    if (driver_init() != 0) {
        fprintf(stderr, "Failed to initialize driver subsystem\n");
        return 1;
    }

    // Create the character driver
    Driver *driver = char_driver_create();
    if (!driver) {
        fprintf(stderr, "Failed to create character driver\n");
        driver_cleanup();
        return 1;
    }

    // Print driver information
    printf("\nDriver Information:\n");
    printf("------------------\n");
    printf("Name: %s\n", driver_get_name(driver));
    printf("Type: %d\n", driver_get_type(driver));
    printf("Version: %u\n", driver_get_version(driver));
    printf("API Version: %u\n", driver_get_api_version(driver));
    printf("Description: %s\n", driver_get_description(driver));
    printf("Author: %s\n", driver_get_author(driver));
    printf("License: %s\n", driver_get_license(driver));

    // Get driver operations
    DriverOps *ops = driver_get_ops(driver);
    if (!ops) {
        fprintf(stderr, "Failed to get driver operations\n");
        driver_cleanup();
        return 1;
    }

    // Open the driver
    if (ops->open(driver_get_driver_data(driver), 0) != 0) {
        fprintf(stderr, "Failed to open driver\n");
        driver_cleanup();
        return 1;
    }

    // Test write operation
    const char *test_data = "Hello, Driver World!\n";
    ssize_t written = ops->write(driver_get_driver_data(driver), test_data, strlen(test_data));
    if (written < 0) {
        fprintf(stderr, "Failed to write to driver\n");
        ops->close(driver_get_driver_data(driver));
        driver_cleanup();
        return 1;
    }
    printf("\nWritten %zd bytes to driver\n", written);

    // Test read operation
    char read_buffer[TEST_BUFFER_SIZE];
    ssize_t read = ops->read(driver_get_driver_data(driver), read_buffer, TEST_BUFFER_SIZE - 1);
    if (read < 0) {
        fprintf(stderr, "Failed to read from driver\n");
        ops->close(driver_get_driver_data(driver));
        driver_cleanup();
        return 1;
    }
    read_buffer[read] = '\0';
    printf("Read %zd bytes from driver: %s", read, read_buffer);

    // Test IOCTL operations
    size_t buffer_size;
    if (ops->ioctl(driver_get_driver_data(driver), 0x2, &buffer_size) == 0) {
        printf("\nBuffer size: %zu bytes\n", buffer_size);
    }

    size_t available_space;
    if (ops->ioctl(driver_get_driver_data(driver), 0x3, &available_space) == 0) {
        printf("Available space: %zu bytes\n", available_space);
    }

    // Test buffer clear
    if (ops->ioctl(driver_get_driver_data(driver), 0x1, NULL) == 0) {
        printf("Buffer cleared\n");
    }

    // Close the driver
    if (ops->close(driver_get_driver_data(driver)) != 0) {
        fprintf(stderr, "Failed to close driver\n");
        driver_cleanup();
        return 1;
    }

    // Cleanup
    driver_cleanup();
    printf("\nTest completed successfully\n");

    return 0;
} 