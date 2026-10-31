#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "asgn1_ioctl.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <reset|stats|mode0|mode1|version|invalid>\n", argv[0]);
        return 1;
    }

    int fd = open("/dev/asgn1", O_RDWR);
    if (fd < 0) {
        perror("open /dev/asgn1");
        return 1;
    }

    if (strcmp(argv[1], "reset") == 0) {
        if (ioctl(fd, ASGN1_RESET_BUFFER) == 0)
            printf("[IOCTL] ASGN1_RESET_BUFFER: Buffer and counters reset OK.\n");
        else
            perror("ioctl RESET_BUFFER");
    } else if (strcmp(argv[1], "stats") == 0) {
        struct asgn1_stats st;
        if (ioctl(fd, ASGN1_GET_STATS, &st) == 0) {
            printf("[IOCTL] ASGN1_GET_STATS:\n");
            printf("  - open_count      : %d\n", st.open_count);
            printf("  - write_count     : %d\n", st.write_count);
            printf("  - read_count      : %d\n", st.read_count);
            printf("  - buffer_len      : %d\n", st.buffer_len);
            printf("  - last_write_size : %d\n", st.last_write_size);
        } else {
            perror("ioctl GET_STATS");
        }
    } else if (strcmp(argv[1], "mode0") == 0 || strcmp(argv[1], "mode1") == 0) {
        int mode = (strcmp(argv[1], "mode0") == 0) ? ASGN1_MODE_KERNEL : ASGN1_MODE_ECHO;
        if (ioctl(fd, ASGN1_SET_MODE, &mode) == 0)
            printf("[IOCTL] ASGN1_SET_MODE: Switched to mode %d (%s)\n",
                   mode, mode == 0 ? "KERNEL MODE" : "ECHO MODE");
        else
            perror("ioctl SET_MODE");
    } else if (strcmp(argv[1], "version") == 0) {
        char ver[ASGN1_VERSION_LEN];
        if (ioctl(fd, ASGN1_GET_VERSION, ver) == 0)
            printf("[IOCTL] ASGN1_GET_VERSION: %s\n", ver);
        else
            perror("ioctl GET_VERSION");
    } else if (strcmp(argv[1], "invalid") == 0) {
        int ret = ioctl(fd, _IO('A', 99));
        printf("[IOCTL] Invalid command test returned: %d (expected -1 / ENOTTY)\n", ret);
    }

    close(fd);
    return 0;
}
