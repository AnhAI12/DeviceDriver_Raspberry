obj-m:=driver_I2C.o
KDIR :=/lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules