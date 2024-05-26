obj-m:=ds3231Dev.o
KDIR :=/lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules