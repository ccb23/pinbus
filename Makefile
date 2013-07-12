ifeq ($(KERNELRELEASE),)
#Outside Kbuild Part

KDIR ?= /lib/modules/`uname -r`/build
$(info Running Makefile outside Kbuild-System)

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c modules.order Module.symvers .tmp_versions

else
#Kbuild Part

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m :=    pinbus.o

endif

