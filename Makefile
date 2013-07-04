ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /usr/src/raspberry_kernel/linux-source-3.6.11+
PWD := $(shell pwd)

.PHONY: build clean

mod:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c

load:
	-rmmod pinbus 
	insmod pinbus.ko
	-mknod /dev/pinbus c `grep pinbus /proc/devices | cut -d ' ' -f 1` 0
else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m :=    pinbus.o

endif
