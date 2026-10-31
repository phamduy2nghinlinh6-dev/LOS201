KDIR := /home/duy/embedded_lab1/lab1/kernel/linux-5.15
ARCH := arm
CROSS_COMPILE := arm-linux-gnueabihf-
ccflags-y += -march=armv7-a

obj-m += asgn1_driver.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) KCFLAGS="-march=armv7-a" modules
	$(CROSS_COMPILE)gcc -march=armv7-a -static -o ioctl_helper ioctl_helper.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -f ioctl_helper
