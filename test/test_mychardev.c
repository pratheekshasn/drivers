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

    close(fd);
    return 0;
}
