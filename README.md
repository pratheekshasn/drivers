# Character Device Driver

A simple Linux kernel module that implements a character device driver with a corresponding test program.

## Project Structure

```
.
├── Makefile              # Build configuration
├── README.md             # This file
├── src/                  # Source files
│   └── mychardev.c       # Kernel module source code
├── test/                 # Test files
│   └── test_mychardev.c  # User-space test program
└── build/                # Build artifacts (generated)
    ├── mychardev.ko      # Compiled kernel module
    ├── test_mychardev    # Test executable
    └── ...               # Other build files
```

## Prerequisites

- Linux development environment
- Kernel headers for your running kernel
- GCC compiler
- Make utility

### Installing Prerequisites

On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

On CentOS/RHEL/Fedora:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install kernel-devel-$(uname -r)
```

## Building

### Build Everything
```bash
make
```
This builds both the kernel module and the test program.

### Build Only the Kernel Module
```bash
make module
```

### Build Only the Test Program
```bash
make test
```

### Clean Build Artifacts
```bash
make clean
```

## Installation and Usage

### 1. Load the Kernel Module
```bash
make install
# or manually:
sudo insmod build/mychardev.ko
```

### 2. Check Module Information
```bash
make info
# or manually:
modinfo build/mychardev.ko
```

### 3. Verify Module is Loaded
```bash
lsmod | grep mychardev
```

### 4. Run the Test Program
```bash
sudo ./build/test_mychardev
```

### 5. Remove the Module
```bash
make remove
# or manually:
sudo rmmod mychardev
```

## Makefile Targets

| Target    | Description                              |
|-----------|------------------------------------------|
| `all`     | Build both kernel module and test program (default) |
| `module`  | Build only the kernel module           |
| `test`    | Build only the test program             |
| `clean`   | Remove all build artifacts             |
| `install` | Load the kernel module                  |
| `remove`  | Unload the kernel module               |
| `info`    | Display module information              |

## Build Output

All build artifacts are placed in the `build/` directory:

- **`mychardev.ko`** - The compiled kernel module
- **`test_mychardev`** - The test executable
- **`*.o`, `*.mod*`** - Intermediate object files
- **`Module.symvers`**, **`modules.order`** - Kernel build metadata

The source directories (`src/` and `test/`) remain clean and contain only source code.

## Development Workflow

1. **Edit source files** in `src/` or `test/`
2. **Build** with `make`
3. **Test** by loading the module and running the test program
4. **Debug** using kernel logs: `dmesg | tail`
5. **Clean up** with `make clean` when needed

## Troubleshooting

### Build Errors
- Ensure kernel headers are installed for your running kernel
- Check that you have sufficient permissions
- Verify GCC and make are installed

### Module Loading Issues
- Check `dmesg` for kernel messages
- Ensure the module isn't already loaded: `lsmod | grep mychardev`
- Verify you're running as root/sudo for module operations

### Permission Errors
- Module loading/unloading requires root privileges
- Use `sudo` for `make install`, `make remove`, and running test programs

## Example Session

```bash
# Build everything
make

# Load the module
make install

# Get hold of the major number that the module was registered with
sudo dmesg | tail -5

sudo mknod /dev/mychardev c <major_number> 0

# Check it's loaded
lsmod | grep mychardev

# Run the test
sudo ./build/test_mychardev

# Check kernel logs
dmesg | tail

# Unload the module
make remove

# Clean up build files
make clean
```

## Notes

- The kernel module creates a character device that can be accessed from user space
- The test program demonstrates how to interact with the device
- All build artifacts are isolated in the `build/` directory
- The project uses the Linux kernel build system (kbuild) for proper module compilation