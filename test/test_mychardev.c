#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("/dev/mychardev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // Write initial data to device
    char write_buf[] = "Hello from user space!";
    if (write(fd, write_buf, strlen(write_buf)) < 0) {
        perror("Failed to write");
        close(fd);
        return 1;
    }

    struct pollfd fds = {
        .fd = fd,
        .events = POLLIN,
    };

    printf("Waiting for device data (simulated interrupt)...\n");

    int ret = poll(&fds, 1, 5000); // 5000 ms timeout
    if (ret == 0) {
        printf("Timeout waiting for data.\n");
    } else if (ret < 0) {
        perror("poll");
    } else {
        if (fds.revents & POLLIN) {
            char buf[256];
            int bytes_read = read(fd, buf, sizeof(buf) - 1);
            if (bytes_read < 0) {
                perror("Failed to read");
            } else {
                buf[bytes_read] = '\0';
                printf("Received data: %s\n", buf);
            }
        }
    }

    close(fd);
    return 0;
}
