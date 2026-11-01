CC_ARM  := arm-linux-gnueabihf-gcc
CC_HOST := gcc
CFLAGS  := -Wall -Wextra -g -O2 -pthread
ARM_FPU := -march=armv7-a -mfpu=neon-vfpv3 -mfloat-abi=hard
LDFLAGS := -pthread -lrt -lm

SRCS := sms_app.c circular_buffer.c sensor_thread.c processor_thread.c alert_thread.c logger_thread.c

all: arm host

arm:
	$(CC_ARM) $(CFLAGS) $(ARM_FPU) -static -o sms_app $(SRCS) $(LDFLAGS)

host:
	$(CC_HOST) $(CFLAGS) -o sms_app_x86 $(SRCS) $(LDFLAGS)

clean:
	rm -f sms_app sms_app_x86 *.o
