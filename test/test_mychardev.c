// Moved from project root to test/
// Moved from project root to test/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("/dev/mychardev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    char write_buf[] = "Hello from user space!";
    if (write(fd, write_buf, strlen(write_buf)) < 0) {
        perror("Failed to write");
        close(fd);
        return 1;
    }

    char read_buf[256];
    int bytes_read = read(fd, read_buf, sizeof(read_buf) - 1);
    if (bytes_read < 0) {
        perror("Failed to read");
        close(fd);
        return 1;
    }
    read_buf[bytes_read] = '\0';
    printf("Read from device: %s\n", read_buf);

    char write_buf_[] = "Bye from user space!";
    if (write(fd, write_buf_, strlen(write_buf_)) < 0) {
        perror("Failed to write");
        close(fd);
        return 1;
    }

    char read_buf_[256];
    bytes_read = read(fd, read_buf_, sizeof(read_buf_) - 1);
    if (bytes_read < 0) {
        perror("Failed to read");
        close(fd);
        return 1;
    }
    read_buf_[bytes_read] = '\0';
    printf("Read from device: %s\n", read_buf_);

    close(fd);
    return 0;
}
