#include<linux/module.h>
#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/kernel.h>
#include<linux/fs.h> // file operation
#include<linux/proc_fs.h>
#include<linux/types.h> //dev_t


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driverADC_open,
	.release = driverADC_close,
	.read = driverADC_read,
	.write = driverADC_write
};

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

static int driverADC_init(void){
	printk("Init driver ADC \n");
	
	return 0;
}

static int driverADC_exit(void){
	printk("close driver ADC \n");
	
	return 0;
}

module_init(driverADC_init);
module_exit(driverADC_exit);