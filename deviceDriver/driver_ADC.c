#include<linux/module.h>
#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/kernel.h>
#include<linux/fs.h> // file operation
#include<linux/proc_fs.h>
#include<linux/types.h> //dev_t
#include<linux/device.h>

#define DRIVER_NAME "driver_ADC"
#define DRIVER_CLASS "MyModuleClass"

static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;	//why need?


static int driverADC_open(struct inode *device_file, struct file *instance){
	printk("device open was called\n");
	
	return 0;
}

static int driverADC_close(struct inode *device_file, struct file *instance){
	printk("device open was called\n");
	
	return 0;
}

static ssize_t driverADC_read(struct file *File, char *user_buff, size_t size, loff_t *offs){
	printk("read ADC value\n");
	
	return 0;
}

static ssize_t driverADC_write(struct file *File, const char *user_buff, size_t size, loff_t *offs){
	printk("write ADC value\n");
	
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driverADC_open,
	.release = driverADC_close,
	.read = driverADC_read,
	.write = driverADC_write
};

static int driverADC_init(void){
	printk("Init driver ADC \n");
	
	//register driver
	if( (alloc_chrdev_region(&my_device_nr,0,1,DRIVER_NAME) <0 ))
	{ printk("Device register fail\n");
		return -1;
	}
	printk("My driver: major = %d, minor = %d \n", my_device_nr>>20 , my_device_nr & 0xfffff);
	
	// create class
	if( (my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL)
	{
		printk("Device class not be created\n");
		unregister_chrdev_region(my_device_nr,1); return -1;
	}
	
	//create device file
	if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) ==NULL)
	{
		printk("can not create device file \n");
		class_destroy(my_class); return -1;
	}
	
	cdev_init(&my_device, &fops);
	//register device
	if(cdev_add(&my_device, my_device_nr, 1)==1){
		printk("registering device fail\n");
		device_destroy(my_class, my_device_nr); return -1;
	}
	return 0;
}

static int driverADC_exit(void){
	printk("close driver ADC \n");
	
	cdev_del(&my_device); //
	device_destroy(my_class, my_device_nr); // delete device
	class_destroy(my_class); //delete class
	unregister_chrdev_region(my_device_nr,1); //delete minor
	return 0;
}

module_init(driverADC_init);
module_exit(driverADC_exit);