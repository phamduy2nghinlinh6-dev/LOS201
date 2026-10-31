#ifndef ASGN1_IOCTL_H
#define ASGN1_IOCTL_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif

/* Magic number định danh riêng cho driver Assignment 1 */
#define ASGN1_MAGIC 'A'

/* Các chế độ hoạt động của driver */
#define ASGN1_MODE_KERNEL 0  /* Tự động thêm tiền tố "[KERNEL] " khi write */
#define ASGN1_MODE_ECHO   1  /* Lưu dữ liệu nguyên bản (mặc định) */

/* Kích thước tối đa của chuỗi version */
#define ASGN1_VERSION_LEN 64

/* Cấu trúc thống kê trả về cho lệnh ASGN1_GET_STATS */
struct asgn1_stats {
    int open_count;
    int write_count;
    int read_count;
    int buffer_len;
    int last_write_size;
};

/* Định nghĩa 4 lệnh ioctl theo chuẩn macro của Linux Kernel */
#define ASGN1_RESET_BUFFER _IO(ASGN1_MAGIC, 1)
#define ASGN1_GET_STATS    _IOR(ASGN1_MAGIC, 2, struct asgn1_stats)
#define ASGN1_SET_MODE     _IOW(ASGN1_MAGIC, 3, int)
#define ASGN1_GET_VERSION  _IOR(ASGN1_MAGIC, 4, char[ASGN1_VERSION_LEN])

#endif /* ASGN1_IOCTL_H */
