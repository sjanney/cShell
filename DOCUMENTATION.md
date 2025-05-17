# VirtualShell OS Documentation

## Architecture Overview

VirtualShell OS is implemented as a modular system with several key components:

### 1. Core Shell Engine
The shell engine is responsible for:
- Command line input processing
- Command parsing and execution
- I/O redirection handling
- Background process management
- Job control

### 2. Virtual OS Components

#### 2.1 Process Management
- **Data Structures**:
  ```c
  typedef struct {
      int pid;
      char name[MAX_LINE];
      int status;
      int priority;
      long memory_usage;
      time_t start_time;
      char owner[MAX_LINE];
  } VProcess;
  ```
- **Key Functions**:
  - `create_process()`: Creates a new virtual process
  - `terminate_process()`: Terminates a running process
  - `show_processes()`: Displays process information

#### 2.2 Memory Management
- **Data Structures**:
  ```c
  typedef struct {
      int id;
      char process_name[MAX_LINE];
      size_t size;
      void *address;
      int is_allocated;
  } MemoryBlock;
  ```
- **Key Functions**:
  - `allocate_memory()`: Allocates memory for a process
  - `free_memory()`: Frees allocated memory
  - `show_memory_usage()`: Displays memory usage

#### 2.3 Network Interface Simulation
- **Data Structures**:
  ```c
  typedef struct {
      char name[MAX_LINE];
      char ip_address[16];
      char mac_address[18];
      int is_up;
      long bytes_sent;
      long bytes_received;
  } NetworkInterface;
  ```
- **Key Functions**:
  - `init_network_interfaces()`: Initializes network interfaces
  - `update_network_stats()`: Updates network statistics
  - `show_network_status()`: Displays network status

#### 2.4 Device Management
- **Data Structures**:
  ```c
  typedef struct {
      char name[MAX_LINE];
      char type[MAX_LINE];
      int status;
      char driver[MAX_LINE];
      long last_access;
  } Device;
  ```
- **Key Functions**:
  - `init_devices()`: Initializes virtual devices
  - `update_device_status()`: Updates device status
  - `show_devices()`: Displays device information

#### 2.5 Virtual Filesystem
- **Data Structures**:
  ```c
  typedef struct {
      char name[MAX_LINE];
      char content[MAX_LINE * 10];
      int size;
      char owner[MAX_LINE];
      char permissions[10];
      time_t created;
      time_t modified;
  } VFile;

  typedef struct {
      char name[MAX_LINE];
      VFile *files[MAX_FILES];
      int file_count;
      char owner[MAX_LINE];
      char permissions[10];
      time_t created;
  } VDirectory;
  ```
- **Key Functions**:
  - `create_vfile()`: Creates a virtual file
  - `create_vdir()`: Creates a virtual directory
  - `list_vfs()`: Lists filesystem contents

#### 2.6 User Management
- **Data Structures**:
  ```c
  typedef struct {
      char username[MAX_LINE];
      char password[MAX_LINE];
      int uid;
      int gid;
      char home_dir[MAX_LINE];
      char shell[MAX_LINE];
  } VUser;
  ```
- **Key Functions**:
  - `handle_user_command()`: Processes user management commands
  - `show_user_info()`: Displays user information

## Command Reference

### Basic Shell Commands

#### cd
```bash
cd [directory]
```
Changes the current working directory.

#### pwd
```bash
pwd
```
Prints the current working directory.

#### ls
```bash
ls [options]
```
Lists directory contents.

#### echo
```bash
echo [text]
```
Prints the specified text.

### Process Management Commands

#### process list
```bash
process list
```
Lists all running processes.

#### process create
```bash
process create <name> [priority]
```
Creates a new process with optional priority.

#### process kill
```bash
process kill <pid>
```
Terminates a process by PID.

### Memory Management Commands

#### memory list
```bash
memory list
```
Shows current memory usage.

#### memory alloc
```bash
memory alloc <process> <size>
```
Allocates memory for a process.

#### memory free
```bash
memory free <block_id>
```
Frees allocated memory block.

### Network Management Commands

#### network list
```bash
network list
```
Lists all network interfaces.

#### network stats
```bash
network stats
```
Shows network statistics.

### Device Management Commands

#### device list
```bash
device list
```
Lists all devices.

#### device status
```bash
device status
```
Shows device status.

## Error Handling

The system implements comprehensive error handling for:
- Invalid commands
- File operations
- Memory allocation
- Process management
- Network operations
- Device operations

## Testing

### Running Tests
```bash
make test
```

### Test Categories
1. Process Management Tests
2. Memory Management Tests
3. Network Interface Tests
4. Device Management Tests
5. Virtual Filesystem Tests
6. User Management Tests
7. Command Parsing Tests
8. History Management Tests
9. Job Control Tests

## Development Guidelines

### Code Style
- Use meaningful variable and function names
- Include comments for complex logic
- Follow C programming best practices
- Maintain modular design

### Adding New Features
1. Create appropriate data structures
2. Implement core functionality
3. Add command handlers
4. Update documentation
5. Write tests
6. Update Makefile if necessary

### Debugging
- Use the test suite for debugging
- Check error messages
- Verify data structures
- Monitor system state

## Performance Considerations

### Memory Usage
- Monitor memory allocation
- Free unused resources
- Implement proper cleanup

### Process Management
- Limit maximum processes
- Monitor process states
- Clean up terminated processes

### File System
- Implement efficient file operations
- Monitor file system size
- Handle file permissions properly

## Security Considerations

### User Management
- Implement proper authentication
- Manage user permissions
- Secure sensitive data

### File System
- Implement proper access control
- Validate file operations
- Protect system files

### Process Management
- Validate process operations
- Implement proper process isolation
- Monitor process permissions 