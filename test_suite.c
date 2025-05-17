#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "main.c"

// Test utilities
#define TEST_PASS() printf("✅ Test passed: %s\n", __func__)
#define TEST_FAIL() printf("❌ Test failed: %s\n", __func__)

// Test function declarations
void test_process_management();
void test_memory_management();
void test_network_interfaces();
void test_device_management();
void test_virtual_filesystem();
void test_user_management();
void test_command_parsing();
void test_history_management();
void test_job_control();

// Test runner
void run_all_tests() {
    printf("\n=== Running VirtualShell OS Test Suite ===\n\n");
    
    test_process_management();
    test_memory_management();
    test_network_interfaces();
    test_device_management();
    test_virtual_filesystem();
    test_user_management();
    test_command_parsing();
    test_history_management();
    test_job_control();
    
    printf("\n=== Test Suite Complete ===\n");
}

// Process Management Tests
void test_process_management() {
    printf("\nTesting Process Management...\n");
    
    // Test process creation
    create_process("test_process", 1);
    assert(process_count > 0);
    assert(strcmp(processes[process_count-1]->name, "test_process") == 0);
    assert(processes[process_count-1]->priority == 1);
    assert(processes[process_count-1]->status == 0);
    
    // Test process termination
    int pid = processes[process_count-1]->pid;
    terminate_process(pid);
    assert(processes[process_count-1]->status == 2);
    
    TEST_PASS();
}

// Memory Management Tests
void test_memory_management() {
    printf("\nTesting Memory Management...\n");
    
    // Test memory allocation
    allocate_memory("test_process", 1024);
    assert(memory_block_count > 0);
    assert(memory_blocks[memory_block_count-1]->size == 1024);
    assert(memory_blocks[memory_block_count-1]->is_allocated == 1);
    
    // Test memory freeing
    int block_id = memory_blocks[memory_block_count-1]->id;
    free_memory(block_id);
    assert(memory_blocks[block_id]->is_allocated == 0);
    
    TEST_PASS();
}

// Network Interface Tests
void test_network_interfaces() {
    printf("\nTesting Network Interfaces...\n");
    
    // Test network interface initialization
    assert(network_interface_count > 0);
    assert(network_interfaces[0]->is_up == 1);
    
    // Test network statistics
    long initial_sent = network_interfaces[0]->bytes_sent;
    update_network_stats();
    assert(network_interfaces[0]->bytes_sent > initial_sent);
    
    TEST_PASS();
}

// Device Management Tests
void test_device_management() {
    printf("\nTesting Device Management...\n");
    
    // Test device initialization
    assert(device_count > 0);
    assert(devices[0]->status == 1);
    
    // Test device status update
    update_device_status();
    assert(devices[0]->last_access > 0);
    
    TEST_PASS();
}

// Virtual Filesystem Tests
void test_virtual_filesystem() {
    printf("\nTesting Virtual Filesystem...\n");
    
    // Test directory creation
    create_vdir("/test_dir");
    assert(vfs_dirs[0] != NULL);
    assert(strcmp(vfs_dirs[0]->name, "/test_dir") == 0);
    
    // Test file creation
    create_vfile("/test_file", "test content");
    assert(vfs_files[0] != NULL);
    assert(strcmp(vfs_files[0]->name, "/test_file") == 0);
    assert(strcmp(vfs_files[0]->content, "test content") == 0);
    
    TEST_PASS();
}

// User Management Tests
void test_user_management() {
    printf("\nTesting User Management...\n");
    
    // Test default user
    assert(users[0] != NULL);
    assert(strcmp(users[0]->username, "root") == 0);
    assert(users[0]->uid == 0);
    
    TEST_PASS();
}

// Command Parsing Tests
void test_command_parsing() {
    printf("\nTesting Command Parsing...\n");
    
    char *args[MAX_ARGS];
    char test_cmd[] = "ls -l /home";
    
    parse_command(test_cmd, args);
    assert(strcmp(args[0], "ls") == 0);
    assert(strcmp(args[1], "-l") == 0);
    assert(strcmp(args[2], "/home") == 0);
    
    TEST_PASS();
}

// History Management Tests
void test_history_management() {
    printf("\nTesting History Management...\n");
    
    // Test history addition
    add_to_history("test command");
    assert(history_count > 0);
    assert(strcmp(history[history_count-1], "test command") == 0);
    
    TEST_PASS();
}

// Job Control Tests
void test_job_control() {
    printf("\nTesting Job Control...\n");
    
    // Test job creation
    jobs[0].pid = 1234;
    jobs[0].status = 0;
    jobs[0].job_id = 1;
    strcpy(jobs[0].cmd, "test_job");
    job_count = 1;
    
    assert(job_count == 1);
    assert(jobs[0].pid == 1234);
    assert(jobs[0].status == 0);
    
    TEST_PASS();
}

int main() {
    // Initialize the virtual environment
    init_virtual_env();
    
    // Run all tests
    run_all_tests();
    
    return 0;
} 