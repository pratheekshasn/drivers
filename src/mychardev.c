// Moved from project root to src/
// Moved from project root to src/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"
#define BUFFER_SIZE 256

static int major_num;
static char device_buffer[BUFFER_SIZE];
static int open_count = 0;

static int dev_open(struct inode *inode, struct file *file) {
    open_count++;
    printk(KERN_INFO "mychardev: Device opened %d times\n", open_count);
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev: Device closed\n");
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    int bytes_to_read = BUFFER_SIZE - *offset;
    if (bytes_to_read > len) bytes_to_read = len;
    if (bytes_to_read == 0) return 0;
    if (copy_to_user(buf, device_buffer + *offset, bytes_to_read)) {
        return -EFAULT;
    }
    *offset += bytes_to_read;
    printk(KERN_INFO "mychardev: Sent %d bytes to the user\n", bytes_to_read);
    return bytes_to_read;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    int bytes_to_write = len;
    if (bytes_to_write > BUFFER_SIZE - 1) bytes_to_write = BUFFER_SIZE - 1;
    if (copy_from_user(device_buffer, buf, bytes_to_write)) {
        return -EFAULT;
    }
    device_buffer[bytes_to_write] = '\0';
    printk(KERN_INFO "mychardev: Received %d bytes from the user\n", bytes_to_write);
    return bytes_to_write;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};

static int __init mychardev_init(void) {
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        printk(KERN_ALERT "mychardev failed to register a major number\n");
        return major_num;
    }
    printk(KERN_INFO "mychardev: registered with major number %d\n", major_num);
    return 0;
}

static void __exit mychardev_exit(void) {
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "mychardev: unregistered module\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Linux Character Device Driver");
MODULE_VERSION("0.1");

module_init(mychardev_init);
module_exit(mychardev_exit);
