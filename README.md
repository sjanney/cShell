# VirtualShell OS

A sophisticated virtual operating system shell implementation in C that simulates various OS components and provides a rich command-line interface.

## Features

### Core Shell Features
- Command-line interface with history support
- Command parsing and execution
- I/O redirection (`>`, `>>`, `<`)
- Background process support (`&`)
- Job control (fg, bg, kill)
- Command history navigation

### Virtual OS Components
1. **Process Management**
   - Process creation and termination
   - Priority-based scheduling
   - Process status tracking
   - Memory usage monitoring

2. **Memory Management**
   - Memory allocation and deallocation
   - Memory block tracking
   - Process memory association

3. **Network Interface Simulation**
   - Virtual network interfaces
   - IP and MAC address management
   - Network statistics tracking
   - Interface status monitoring

4. **Device Management**
   - Virtual device simulation
   - Device status tracking
   - Driver management
   - Last access monitoring

5. **Virtual Filesystem**
   - File and directory creation
   - Permission management
   - Ownership tracking
   - Content management

6. **User Management**
   - User account management
   - UID/GID system
   - Home directory support
   - Shell configuration

## Building and Running

### Prerequisites
- GCC compiler
- Make
- Standard C library

### Compilation
```bash
cd cShell
make
```

### Running
```bash
./app
```

### Running Tests
```bash
make test
```

## Available Commands

### Basic Shell Commands
- `cd [directory]` - Change directory
- `pwd` - Print working directory
- `ls [options]` - List directory contents
- `echo [text]` - Print text
- `history` - Show command history
- `help` - Show help message
- `exit` - Exit the shell

### Process Management
- `process list` - List all processes
- `process create <name> [priority]` - Create a new process
- `process kill <pid>` - Terminate a process

### Memory Management
- `memory list` - Show memory usage
- `memory alloc <process> <size>` - Allocate memory
- `memory free <block_id>` - Free memory block

### Network Management
- `network list` - Show network interfaces
- `network stats` - Update and show network statistics

### Device Management
- `device list` - List all devices
- `device status` - Show device status

### Virtual Filesystem
- `vfs list` - List filesystem contents
- `vfs create <name> <content>` - Create a file
- `vfs delete <name>` - Delete a file

### User Management
- `user list` - List all users
- `user info` - Show current user information

## Project Structure

```
cShell/
├── main.c           # Main shell implementation
├── test_suite.c     # Test suite
├── Makefile         # Build configuration
└── README.md        # This file
```

## Testing

The project includes a comprehensive test suite that verifies the functionality of all major components. Run the tests using:

```bash
make test
```

The test suite covers:
- Process management
- Memory management
- Network interfaces
- Device management
- Virtual filesystem
- User management
- Command parsing
- History management
- Job control

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by Unix/Linux shell implementations
- Built for educational purposes
- Designed to demonstrate OS concepts 