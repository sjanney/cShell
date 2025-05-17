# cShell

A modern, feature-rich shell with AI capabilities built in C.

## Features

- 🎨 Beautiful colored interface
- 🤖 AI-powered command suggestions and explanations
- 📝 Command history
- 🔄 Process management
- 📂 File system operations
- 🌍 Environment variable management
- 🎯 Built-in commands
- 🛡️ Signal handling

## Prerequisites

- GCC compiler
- Make
- libreadline
- libcurl
- OpenAI API key (for AI features)

## Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/cShell.git
cd cShell
```

2. Install dependencies:

On Ubuntu/Debian:
```bash
sudo apt-get install build-essential libreadline-dev libcurl4-openssl-dev
```

On macOS:
```bash
brew install readline curl
```

3. Build the shell:
```bash
make
```

## Usage

1. Start the shell:
```bash
./bin/cshell
```

2. Set up OpenAI API key (for AI features):
```bash
export OPENAI_API_KEY="your-api-key-here"
```

## Available Commands

### Basic Commands
- `help` - Display help information
- `exit` - Exit the shell
- `clear` - Clear the screen
- `ls` - List directory contents
- `cd` - Change directory
- `pwd` - Print working directory

### File Operations
- `mkdir` - Create a new directory
- `rmdir` - Remove an empty directory
- `touch` - Create an empty file
- `rm` - Remove a file
- `cat` - Display file contents
- `echo` - Display a line of text

### Process Management
- `ps` - List processes
- `kill` - Terminate a process
- `bg` - Resume a stopped job in background
- `fg` - Resume a stopped job in foreground
- `jobs` - List background jobs

### Environment Variables
- `env` - Display environment variables
- `export` - Set an environment variable
- `unset` - Remove an environment variable

### AI Commands
- `ai help` - Show AI command help
- `ai explain <command>` - Explain what a command does
- `ai suggest <description>` - Get command suggestions
- `ai learn <input> <feedback>` - Provide feedback to AI

## Building from Source

1. Clean build:
```bash
make clean
make
```

2. Debug build:
```bash
make debug
```

3. Test build:
```bash
make test
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- GNU Readline for command line editing
- OpenAI for AI capabilities
- All contributors who have helped shape this project 