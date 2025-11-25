#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/poll.h>

#define DEVICE_NAME "mychardev"
#define BUFFER_SIZE 256

// IOCTL definitions
#define MYCHARDEV_IOCTL_BASE 'W'
#define MYCHARDEV_IOCTL_RESET _IO(MYCHARDEV_IOCTL_BASE, 0)
#define MYCHARDEV_IOCTL_GET_VAL _IOR(MYCHARDEV_IOCTL_BASE, 1, int)

static int major_num;
static char device_buffer[BUFFER_SIZE];
static int open_count = 0;
static int data_len = 0;
static int some_value = 42;

static struct kobject *mykobj;
static int sysfs_value = 100;

// Timer and wait queue for simulating interrupts
static struct timer_list my_timer;
static wait_queue_head_t wait_queue;
static int data_ready_flag = 0;

// Forward declarations
static int dev_open(struct inode *inode, struct file *file);
static int dev_release(struct inode *inode, struct file *file);
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static unsigned int dev_poll(struct file *file, poll_table *wait);

// Timer callback simulating an interrupt every 2 seconds
static void timer_callback(struct timer_list *t) {
    data_ready_flag = 1; // Mark data as ready
    wake_up_interruptible(&wait_queue); // Wake up any waiting readers
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000)); // Re-arm timer
}

// File operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
    .poll = dev_poll,
};

// IOCTL handler
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int val;
    switch (cmd) {
        case MYCHARDEV_IOCTL_RESET:
            some_value = 0;
            printk(KERN_INFO "mychardev: ioctl reset called\n");
            break;
        case MYCHARDEV_IOCTL_GET_VAL:
            val = some_value;
            if (copy_to_user((int __user *)arg, &val, sizeof(int)))
                return -EFAULT;
            printk(KERN_INFO "mychardev: ioctl get val: %d\n", val);
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

// Sysfs show/store functions
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", sysfs_value);
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count) {
    sscanf(buf, "%du", &sysfs_value);
    return count;
}

static struct kobj_attribute my_attribute = __ATTR(sysfs_value, 0660, sysfs_show, sysfs_store);

// Device open - initialize timer and waitqueue
static int dev_open(struct inode *inode, struct file *file) {
    open_count++;
    printk(KERN_INFO "mychardev: Device opened %d times\n", open_count);

    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));

    init_waitqueue_head(&wait_queue);
    data_ready_flag = 0;

    return 0;
}

// Device release - clean up timer
static int dev_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev: Device closed\n");
    del_timer_sync(&my_timer);
    return 0;
}

// Read - only returns data if data_ready_flag is set, resets flag after read
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    int bytes_to_read;

    if (!data_ready_flag)
        return 0; // No new data; signal EOF to user

    bytes_to_read = data_len - *offset;
    if (bytes_to_read > len)
        bytes_to_read = len;
    if (bytes_to_read <= 0)
        return 0;

    if (copy_to_user(buf, device_buffer + *offset, bytes_to_read))
        return -EFAULT;

    *offset += bytes_to_read;
    data_ready_flag = 0; // Reset flag so next read blocks until timer sets again

    printk(KERN_INFO "mychardev: Sent %d bytes to the user\n", bytes_to_read);
    return bytes_to_read;
}

// Write - stores data and resets offset for next read
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    int bytes_to_write = (len > BUFFER_SIZE - 1) ? BUFFER_SIZE - 1 : len;

    if (copy_from_user(device_buffer, buf, bytes_to_write))
        return -EFAULT;

    device_buffer[bytes_to_write] = '\0'; // Null terminate string
    data_len = bytes_to_write;
    *offset = 0;

    printk(KERN_INFO "mychardev: Received %d bytes from the user\n", bytes_to_write);
    return bytes_to_write;
}

// Poll implementation - tells user space whether data is ready
static unsigned int dev_poll(struct file *file, poll_table *wait) {
    poll_wait(file, &wait_queue, wait);

    if (data_ready_flag)
        return POLLIN | POLLRDNORM;

    return 0;
}

// Module init and exit with sysfs setup
static int __init mychardev_init(void) {
    int error;

    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0)
        return major_num;

    mykobj = kobject_create_and_add("mychardev", kernel_kobj);
    if (!mykobj) {
        unregister_chrdev(major_num, DEVICE_NAME);
        return -ENOMEM;
    }

    error = sysfs_create_file(mykobj, &my_attribute.attr);
    if (error) {
        kobject_put(mykobj);
        unregister_chrdev(major_num, DEVICE_NAME);
        return error;
    }

    printk(KERN_INFO "mychardev: module loaded with major %d\n", major_num);
    return 0;
}

static void __exit mychardev_exit(void) {
    sysfs_remove_file(mykobj, &my_attribute.attr);
    kobject_put(mykobj);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "mychardev: module unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Linux Character Device Driver with Timer Interrupt and Polling");
MODULE_VERSION("0.3");

module_init(mychardev_init);
module_exit(mychardev_exit);
