#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include "asgn1_ioctl.h"

#define DEVICE_NAME     "asgn1"
#define CLASS_NAME      "asgn1_class"
#define DRIVER_MAJOR    241
#define BUFFER_SIZE     1024
#define DRIVER_VERSION  "asgn1_driver v1.0 - PhamDangKhanhDuy"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pham Dang Khanh Duy");
MODULE_DESCRIPTION("ASSIGNMENT-01 Task 1: Character Device Driver with ioctl Interface");
MODULE_VERSION("1.0");

/* Biến trạng thái nội bộ của driver */
static int major_number = DRIVER_MAJOR;
static char device_buffer[BUFFER_SIZE];
static int buffer_len = 0;
static int current_mode = ASGN1_MODE_ECHO; /* Mặc định là Mode 1 (Echo) */

/* Bộ đếm thống kê */
static int open_count = 0;
static int write_count = 0;
static int read_count = 0;
static int last_write_size = 0;

static struct class *asgn1_class = NULL;
static struct device *asgn1_device = NULL;

/* Mutex bảo vệ buffer và các biến thống kê khỏi race condition */
static DEFINE_MUTEX(asgn1_mutex);

/**
 * asgn1_open - Được gọi khi tiến trình userspace mở /dev/asgn1
 * @inode: Con trỏ cấu trúc inode của file thiết bị
 * @file: Con trỏ cấu trúc file đang mở
 * Return: 0 nếu thành công
 */
static int asgn1_open(struct inode *inode, struct file *file)
{
    mutex_lock(&asgn1_mutex);
    open_count++;
    mutex_unlock(&asgn1_mutex);
    pr_info("asgn1_driver: device opened (open_count=%d)\n", open_count);
    return 0;
}

/**
 * asgn1_release - Được gọi khi tiến trình đóng file descriptor của /dev/asgn1
 * @inode: Con trỏ cấu trúc inode
 * @file: Con trỏ cấu trúc file
 * Return: 0 nếu thành công
 */
static int asgn1_release(struct inode *inode, struct file *file)
{
    pr_info("asgn1_driver: device closed\n");
    return 0;
}

/**
 * asgn1_read - Đọc dữ liệu từ device_buffer trả về cho userspace
 * @file: Con trỏ file đang mở
 * @buf: Địa chỉ vùng nhớ ở userspace
 * @len: Số byte yêu cầu đọc
 * @offset: Vị trí con trỏ đọc hiện tại
 * Return: Số byte thực tế đã đọc, 0 nếu EOF, hoặc mã lỗi âm
 */
static ssize_t asgn1_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    ssize_t bytes_to_read;

    mutex_lock(&asgn1_mutex);

    if (*offset >= buffer_len) {
        mutex_unlock(&asgn1_mutex);
        return 0; /* EOF */
    }

    bytes_to_read = buffer_len - *offset;
    if (len < bytes_to_read)
        bytes_to_read = len;

    if (copy_to_user(buf, device_buffer + *offset, bytes_to_read)) {
        mutex_unlock(&asgn1_mutex);
        pr_err("asgn1_driver: copy_to_user failed in read\n");
        return -EFAULT;
    }

    *offset += bytes_to_read;
    read_count++;
    mutex_unlock(&asgn1_mutex);

    pr_info("asgn1_driver: read %zd bytes (read_count=%d)\n", bytes_to_read, read_count);
    return bytes_to_read;
}

/**
 * asgn1_write - Nhận dữ liệu từ userspace và lưu vào device_buffer theo chế độ
 * @file: Con trỏ file đang mở
 * @buf: Địa chỉ vùng nhớ dữ liệu ở userspace
 * @len: Độ dài dữ liệu ghi xuống
 * @offset: Vị trí ghi
 * Return: Số byte đã nhận từ userspace hoặc mã lỗi âm
 */
static ssize_t asgn1_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    char kbuf[BUFFER_SIZE];
    const char *prefix = "[KERNEL] ";
    size_t prefix_len = strlen(prefix);
    size_t copy_len = len;

    if (copy_len >= BUFFER_SIZE)
        copy_len = BUFFER_SIZE - 1;

    if (copy_from_user(kbuf, buf, copy_len)) {
        pr_err("asgn1_driver: copy_from_user failed in write\n");
        return -EFAULT;
    }
    kbuf[copy_len] = '\0';

    mutex_lock(&asgn1_mutex);

    if (current_mode == ASGN1_MODE_KERNEL) {
        /* Mode 0: Tự động chèn tiền tố "[KERNEL] " vào trước dữ liệu */
        if (prefix_len + copy_len >= BUFFER_SIZE)
            copy_len = BUFFER_SIZE - 1 - prefix_len;
        snprintf(device_buffer, BUFFER_SIZE, "%s%.*s", prefix, (int)copy_len, kbuf);
        buffer_len = strlen(device_buffer);
    } else {
        /* Mode 1 (Echo mode): Lưu nguyên bản */
        memcpy(device_buffer, kbuf, copy_len);
        device_buffer[copy_len] = '\0';
        buffer_len = copy_len;
    }

    write_count++;
    last_write_size = (int)len;
    mutex_unlock(&asgn1_mutex);

    pr_info("asgn1_driver: write %zu bytes in mode %d -> buffer_len=%d\n",
            len, current_mode, buffer_len);
    return len;
}

/**
 * asgn1_ioctl - Xử lý 4 lệnh điều khiển ioctl từ userspace
 * @file: Con trỏ file đang mở
 * @cmd: Mã lệnh ioctl
 * @arg: Tham số truyền từ userspace (giá trị hoặc con trỏ)
 * Return: 0 nếu thành công, mã lỗi âm nếu thất bại (-ENOTTY, -EINVAL, -EFAULT)
 */
static long asgn1_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct asgn1_stats stats;
    char ver_buf[ASGN1_VERSION_LEN];
    int new_mode;

    switch (cmd) {
    case ASGN1_RESET_BUFFER:
        mutex_lock(&asgn1_mutex);
        memset(device_buffer, 0, BUFFER_SIZE);
        buffer_len = 0;
        write_count = 0;
        read_count = 0;
        last_write_size = 0;
        mutex_unlock(&asgn1_mutex);
        pr_info("asgn1_driver: ioctl ASGN1_RESET_BUFFER executed\n");
        break;

    case ASGN1_GET_STATS:
        mutex_lock(&asgn1_mutex);
        stats.open_count = open_count;
        stats.write_count = write_count;
        stats.read_count = read_count;
        stats.buffer_len = buffer_len;
        stats.last_write_size = last_write_size;
        mutex_unlock(&asgn1_mutex);

        if (copy_to_user((struct asgn1_stats __user *)arg, &stats, sizeof(stats))) {
            pr_err("asgn1_driver: copy_to_user failed in ASGN1_GET_STATS\n");
            return -EFAULT;
        }
        pr_info("asgn1_driver: ioctl ASGN1_GET_STATS executed\n");
        break;

    case ASGN1_SET_MODE:
        if (copy_from_user(&new_mode, (int __user *)arg, sizeof(int))) {
            pr_err("asgn1_driver: copy_from_user failed in ASGN1_SET_MODE\n");
            return -EFAULT;
        }
        if (new_mode != ASGN1_MODE_KERNEL && new_mode != ASGN1_MODE_ECHO) {
            pr_warn("asgn1_driver: invalid mode %d requested\n", new_mode);
            return -EINVAL;
        }
        mutex_lock(&asgn1_mutex);
        current_mode = new_mode;
        mutex_unlock(&asgn1_mutex);
        pr_info("asgn1_driver: ioctl ASGN1_SET_MODE set mode=%d\n", current_mode);
        break;

    case ASGN1_GET_VERSION:
        memset(ver_buf, 0, ASGN1_VERSION_LEN);
        strncpy(ver_buf, DRIVER_VERSION, ASGN1_VERSION_LEN - 1);
        if (copy_to_user((char __user *)arg, ver_buf, ASGN1_VERSION_LEN)) {
            pr_err("asgn1_driver: copy_to_user failed in ASGN1_GET_VERSION\n");
            return -EFAULT;
        }
        pr_info("asgn1_driver: ioctl ASGN1_GET_VERSION executed\n");
        break;

    default:
        pr_warn("asgn1_driver: unknown ioctl command 0x%x\n", cmd);
        return -ENOTTY;
    }

    return 0;
}

static const struct file_operations asgn1_fops = {
    .owner          = THIS_MODULE,
    .open           = asgn1_open,
    .release        = asgn1_release,
    .read           = asgn1_read,
    .write          = asgn1_write,
    .unlocked_ioctl = asgn1_ioctl,
};

static int __init asgn1_init(void)
{
    int ret;

    ret = register_chrdev(major_number, DEVICE_NAME, &asgn1_fops);
    if (ret < 0) {
        pr_err("asgn1_driver: failed to register chrdev (err=%d)\n", ret);
        return ret;
    }

    asgn1_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(asgn1_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(asgn1_class);
    }

    asgn1_device = device_create(asgn1_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(asgn1_device)) {
        class_destroy(asgn1_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(asgn1_device);
    }

    memset(device_buffer, 0, BUFFER_SIZE);
    pr_info("asgn1_driver: loaded successfully (major=%d, default_mode=ECHO)\n", major_number);
    return 0;
}

static void __exit asgn1_exit(void)
{
    device_destroy(asgn1_class, MKDEV(major_number, 0));
    class_destroy(asgn1_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("asgn1_driver: unloaded successfully\n");
}

module_init(asgn1_init);
module_exit(asgn1_exit);
