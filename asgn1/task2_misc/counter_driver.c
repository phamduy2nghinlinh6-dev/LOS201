#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define DEVICE_NAME "counter"
#define PROC_NAME   "counter_info"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pham Dang Khanh Duy");
MODULE_DESCRIPTION("ASSIGNMENT-01 Task 2: Misc Device Driver - Atomic Counter");
MODULE_VERSION("1.0");

/* Biến đếm kiểu atomic_t đảm bảo an toàn đa luồng trên hệ thống SMP */
static atomic_t counter_val   = ATOMIC_INIT(0);
static atomic_t total_reads   = ATOMIC_INIT(0);
static atomic_t total_writes  = ATOMIC_INIT(0);

static struct proc_dir_entry *counter_proc_entry = NULL;

/**
 * counter_read - Đọc giá trị hiện tại của counter và tự động tăng thêm 1
 * @file: Con trỏ file đang mở
 * @buf: Buffer ở userspace
 * @count: Số byte yêu cầu đọc
 * @ppos: Con trỏ vị trí offset của file
 * Return: Số byte đã đọc, hoặc 0 khi EOF (*ppos > 0)
 */
static ssize_t counter_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char kbuf[32];
    int cur_val, len;

    /* Xử lý offset để lệnh cat dừng lại sau lần đọc đầu tiên */
    if (*ppos > 0)
        return 0;

    /* Lấy giá trị hiện tại và tự động tăng counter lên 1 */
    cur_val = atomic_fetch_inc(&counter_val);
    atomic_inc(&total_reads);

    len = snprintf(kbuf, sizeof(kbuf), "%d\n", cur_val);
    if (count < len)
        len = count;

    if (copy_to_user(buf, kbuf, len)) {
        pr_err("counter_driver: copy_to_user failed\n");
        return -EFAULT;
    }

    *ppos += len;
    pr_info("counter_driver: read value=%d (new_val=%d, total_reads=%d)\n",
            cur_val, atomic_read(&counter_val), atomic_read(&total_reads));
    return len;
}

/**
 * counter_write - Nhận chuỗi số nguyên dương hoặc '0' từ userspace để cập nhật counter
 * @file: Con trỏ file đang mở
 * @buf: Buffer ở userspace
 * @count: Số byte ghi xuống
 * @ppos: Con trỏ offset
 * Return: Số byte đã xử lý hoặc -EINVAL nếu dữ liệu âm / không phải số
 */
static ssize_t counter_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char kbuf[32];
    int new_val, ret;
    size_t copy_len = count;

    if (copy_len >= sizeof(kbuf))
        copy_len = sizeof(kbuf) - 1;

    if (copy_from_user(kbuf, buf, copy_len)) {
        pr_err("counter_driver: copy_from_user failed\n");
        return -EFAULT;
    }
    kbuf[copy_len] = '\0';

    /* Chuyển đổi chuỗi ASCII sang số nguyên bằng kstrtoint */
    ret = kstrtoint(strim(kbuf), 10, &new_val);
    if (ret < 0 || new_val < 0) {
        pr_warn("counter_driver: invalid write input '%s' (must be non-negative integer)\n", kbuf);
        return -EINVAL;
    }

    atomic_set(&counter_val, new_val);
    atomic_inc(&total_writes);

    pr_info("counter_driver: counter set to %d (total_writes=%d)\n",
            new_val, atomic_read(&total_writes));
    return count;
}

static const struct file_operations counter_fops = {
    .owner = THIS_MODULE,
    .read  = counter_read,
    .write = counter_write,
};

/* Cấu trúc đăng ký Misc Device (Major 10, Minor tự động cấp phát) */
static struct miscdevice counter_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEVICE_NAME,
    .fops  = &counter_fops,
    .mode  = 0666,
};

/* Giao diện /proc/counter_info */
static int counter_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "=== Counter Device Statistics ===\n");
    seq_printf(m, "Device Node         : /dev/%s (Major 10, Minor %d)\n",
               DEVICE_NAME, counter_misc_device.minor);
    seq_printf(m, "Current Counter     : %d\n", atomic_read(&counter_val));
    seq_printf(m, "Total Reads (Inc)   : %d\n", atomic_read(&total_reads));
    seq_printf(m, "Total Writes (Set)  : %d\n", atomic_read(&total_writes));
    return 0;
}

static int counter_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, counter_proc_show, NULL);
}

static const struct proc_ops counter_proc_fops = {
    .proc_open    = counter_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init counter_init(void)
{
    int ret;

    ret = misc_register(&counter_misc_device);
    if (ret) {
        pr_err("counter_driver: misc_register failed (err=%d)\n", ret);
        return ret;
    }

    counter_proc_entry = proc_create(PROC_NAME, 0444, NULL, &counter_proc_fops);
    if (!counter_proc_entry) {
        misc_deregister(&counter_misc_device);
        pr_err("counter_driver: failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("counter_driver: registered /dev/%s (major=10, minor=%d) and /proc/%s\n",
            DEVICE_NAME, counter_misc_device.minor, PROC_NAME);
    return 0;
}

static void __exit counter_exit(void)
{
    if (counter_proc_entry)
        proc_remove(counter_proc_entry);
    misc_deregister(&counter_misc_device);
    pr_info("counter_driver: unregistered successfully\n");
}

module_init(counter_init);
module_exit(counter_exit);
