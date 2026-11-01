#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/ktime.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include "sms_sensor.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pham Dang Khanh Duy");
MODULE_DESCRIPTION("ASSIGNMENT-02: SMS Simulated Sensor Driver");
MODULE_VERSION("1.0");

static DEFINE_MUTEX(sms_mutex);

static unsigned long read_count = 0;
static int min_interval_ms = 100;
static unsigned long boot_jiffies = 0;
static int last_temp_x10 = 250;
static int last_hum_x10  = 600;

static struct proc_dir_entry *sms_proc_entry = NULL;

static int sms_open(struct inode *inode, struct file *file) { return 0; }
static int sms_release(struct inode *inode, struct file *file) { return 0; }

static ssize_t sms_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char kbuf[64];
    unsigned long now_ms;
    u32 rand1, rand2;
    int temp_x10, hum_x10, len;

    if (*ppos > 0)
        return 0;

    now_ms = jiffies_to_msecs(jiffies - boot_jiffies);
    rand1  = get_random_u32();
    rand2  = get_random_u32();

    temp_x10 = 150 + (rand1 % 301);
    hum_x10  = 300 + (rand2 % 601);

    mutex_lock(&sms_mutex);
    read_count++;
    last_temp_x10 = temp_x10;
    last_hum_x10  = hum_x10;
    mutex_unlock(&sms_mutex);

    len = snprintf(kbuf, sizeof(kbuf), "%lu,%d.%d,%d.%d\n",
                   now_ms,
                   temp_x10 / 10, temp_x10 % 10,
                   hum_x10 / 10, hum_x10 % 10);

    if (count < (size_t)len)
        len = count;

    if (copy_to_user(buf, kbuf, len))
        return -EFAULT;

    *ppos += len;
    return len;
}

static long sms_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int interval;

    switch (cmd) {
    case SMS_SET_INTERVAL:
        if (copy_from_user(&interval, (int __user *)arg, sizeof(int)))
            return -EFAULT;
        if (interval < 10 || interval > 60000)
            return -EINVAL;
        mutex_lock(&sms_mutex);
        min_interval_ms = interval;
        mutex_unlock(&sms_mutex);
        pr_info("sms_sensor: sampling interval set to %d ms\n", min_interval_ms);
        break;
    default:
        return -ENOTTY;
    }
    return 0;
}

static const struct file_operations sms_fops = {
    .owner          = THIS_MODULE,
    .open           = sms_open,
    .release        = sms_release,
    .read           = sms_read,
    .unlocked_ioctl = sms_ioctl,
};

static struct miscdevice sms_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = SMS_DEVICE_NAME,
    .fops  = &sms_fops,
    .mode  = 0666,
};

static int sms_proc_show(struct seq_file *m, void *v)
{
    unsigned long uptime_sec = jiffies_to_msecs(jiffies - boot_jiffies) / 1000;

    mutex_lock(&sms_mutex);
    seq_printf(m, "=== SMS Sensor Driver Statistics ===\n");
    seq_printf(m, "read_count       : %lu\n", read_count);
    seq_printf(m, "last_temperature : %d.%d C\n", last_temp_x10 / 10, last_temp_x10 % 10);
    seq_printf(m, "last_humidity    : %d.%d %%\n", last_hum_x10 / 10, last_hum_x10 % 10);
    seq_printf(m, "min_interval_ms  : %d ms\n", min_interval_ms);
    seq_printf(m, "driver_uptime    : %lu seconds\n", uptime_sec);
    mutex_unlock(&sms_mutex);
    return 0;
}

static int sms_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, sms_proc_show, NULL);
}

static const struct proc_ops sms_proc_fops = {
    .proc_open    = sms_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init sms_init(void)
{
    int ret = misc_register(&sms_misc_dev);
    if (ret) {
        pr_err("sms_sensor: misc_register failed (%d)\n", ret);
        return ret;
    }

    sms_proc_entry = proc_create(SMS_PROC_NAME, 0444, NULL, &sms_proc_fops);
    if (!sms_proc_entry) {
        misc_deregister(&sms_misc_dev);
        return -ENOMEM;
    }

    boot_jiffies = jiffies;
    pr_info("sms_sensor: registered /dev/%s (minor=%d) and /proc/%s\n",
            SMS_DEVICE_NAME, sms_misc_dev.minor, SMS_PROC_NAME);
    return 0;
}

static void __exit sms_exit(void)
{
    if (sms_proc_entry)
        proc_remove(sms_proc_entry);
    misc_deregister(&sms_misc_dev);
    pr_info("sms_sensor: unloaded (total reads=%lu)\n", read_count);
}

module_init(sms_init);
module_exit(sms_exit);
