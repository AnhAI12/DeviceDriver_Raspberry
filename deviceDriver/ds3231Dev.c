/*
	
	
	Function: set_time,
				read_time (optional mode read)

	Preference: SMBus, I2C
		https://www.kernel.org/doc/html/v5.5/i2c/smbus-protocol.html
		https://www.kernel.org/doc/html/latest/i2c/writing-clients.html
		https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html
        https://www.kernel.org/doc/html/v4.12/driver-api/i2c.html
        https://linux-kernel-labs.github.io/refs/heads/master/labs/device_model.html
	Function:
		i2c_smbus_write_byte_data(client, reg, value);
		i2c_smbus_read_i2c_block_data(struct i2c_client *client,
                                  u8 command, u8 length, u8 *values);
								  
	Step by step:
		1. Init, Exit device driver
		2. Create a struct I2C device driver
		3. Create char device
		4. Create ioclt
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h> // create device number
#include <linux/device.h>
#include <linux/uaccess.h>
#include<linux/cdev.h>	//assign the identifier to char device
#include<linux/ioctl.h> //ioctl
#include<linux/kernel.h>
#include "ds3231Dev.h"


#define DRIVER_NAME "ds3231_driver"
#define CLASS_NAME "ds3231"
#define DEVICE_NAME "ds3231"

// i2c slave address of the DS3231 chip
#define DS3231_I2C_ADDR             0x68
#define I2C_BUS_AVAILABLE           1


static struct i2c_client *ds3231_client = NULL;        
static struct i2c_adapter *etx_i2c_adapter     = NULL;  // I2C Adapter Structure - /dev/i2c-1
static struct class* ds3231_class = NULL;
static struct device* ds3231_device = NULL;
static struct cdev my_device;
static dev_t myDeviceNr;    //major vs minnor

//union mode{};
#define TIME_MODE 0
#define DAY_MODE 1
#define DATE_MODE 2
#define MONTH_MODE 0
#define YEAR_MODE 0


uint8_t hex2dec(uint8_t data)
{
    uint8_t dec;
    dec=10*(data>>4)+(data & 0x0F);
    return dec;
}


// static int driver_read(struct file *File, int *user_buffer, int mode)
// {
//     uint8_t buf[7];

//     switch (mode)
//     {
//     case TIME_MODE:
//         //second
//         buf[0] = hex2dec(i2c_smbus_read_byte_data(ds3231_client, DS3231_SEC_ADDR));
//         //min
//         buf[1] = hex2dec(i2c_smbus_read_byte_data(ds3231_client, DS3231_MIN_ADDR));
//         //hour
//         buf[2] = hex2dec(i2c_smbus_read_byte_data(ds3231_client, DS3231_HOUR_ADDR));
//         break;

//     default:
//         break;
//     }
//     //truyen dia chi
//      if (copy_to_user(user_buffer, buf, sizeof(buf))) {
//         return -EFAULT;
//     }

//     return 1;
// }

// static int driver_write(struct file *File, int user_buffer[], int mode)
// {
//     uint8_t buf[7];

//     switch (mode)
//     {
//     case TIME_MODE:
//         //second
//         i2c_smbus_write_byte_data(ds3231_client, DS3231_SEC_ADDR, user_buffer[0]);
//         //min
//         i2c_smbus_write_byte_data(ds3231_client, DS3231_MIN_ADDR, user_buffer[1]);
//         //hour
//         i2c_smbus_write_byte_data(ds3231_client, DS3231_HOUR_ADDR, user_buffer[2]);
//         break;

//     default:
//         return 0;
//         break;
//     }
//     return 1;
// }

static void ds3231_write( tm buff){
    //second
    i2c_smbus_write_byte_data(ds3231_client, DS3231_SEC_ADDR, buff.tm_sec);
    //min
    i2c_smbus_write_byte_data(ds3231_client, DS3231_MIN_ADDR, buff.tm_min);
    //hour
    i2c_smbus_write_byte_data(ds3231_client, DS3231_HOUR_ADDR, buff.tm_hour);
    
}

static void ds3231_read( tm *buff){
    //second
    buff->tm_sec = hex2dec(i2c_smbus_read_byte_data(ds3231_client, DS3231_SEC_ADDR));
    //min
    buff->tm_min= hex2dec(i2c_smbus_read_byte_data(ds3231_client, DS3231_MIN_ADDR));
    //hour
    buff->tm_hour = hex2dec(i2c_smbus_read_byte_data(ds3231_client, DS3231_HOUR_ADDR));
}

static long ds3231_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret=-1;
    int data;
    tm buff;

    switch (cmd) {
        case DS3231_IOCTL_WRITE:
            ret = copy_from_user(&buff, (tm*) arg, sizeof(tm));
            ds3231_write(buff);
            break;
        case DS3231_IOCTL_READ:
            ds3231_read( &buff);
            ret = copy_to_user((tm*)arg, &buff, sizeof(tm));
            break;
        default:
            return -EINVAL;
    }

    // if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
    //     return -EFAULT;
    // }

    return 0;
}

static int ds3231_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "ds3231 device opened\n");
    return 0;
}

static int ds3231_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "ds3231 device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ds3231_open,
    .unlocked_ioctl = ds3231_ioctl,
    .release = ds3231_release,
    // .read = driver_read,
    // .write = driver_write,
};

// static int ds3231_probe(struct i2c_client *client, const struct i2c_device_id *id)
// {

// 	// ds3231_client = client;
//     //initialize sensor
//     printk("ds3231 driver probe\n");

//     return 0;
// }

static void ds3231_remove(struct i2c_client *client)
{
    printk(KERN_INFO "ds3231 driver removed\n");

    // Clean up
 
}

//	Struct of I2C device driver

static const struct i2c_device_id ds3231_id[] = {
    { "ds3231", 0 },
    { }
};

/*
** I2C Board Info strucutre
*/
static struct i2c_board_info ds3231_i2c_board_info = {
        I2C_BOARD_INFO(DEVICE_NAME, DS3231_I2C_ADDR)
    };

MODULE_DEVICE_TABLE(i2c, ds3231_id);

static struct i2c_driver ds3231_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
    },
    // .probe      = ds3231_probe,	//bind to device
    .remove     = ds3231_remove,
    .id_table   = ds3231_id,
};

static int __init ds3231_init(void)
{
    printk(KERN_INFO "Initializing DS3231 driver\n");   //abc
    int ret = -1;
    // Allocate Device Nr
    if ( alloc_chrdev_region(&myDeviceNr, 0, 1, DRIVER_NAME) < 0) {
		printk("Device Nr. could not be allocated!\n");
	}
	printk("MyDeviceDriver - Device Nr %d was registered\n", myDeviceNr);

    //create Device Class
    ds3231_class = class_create(CLASS_NAME);
    if (ds3231_class == NULL) {
        unregister_chrdev(myDeviceNr, DRIVER_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return (-1);
    }
    //create device file
    ds3231_device = device_create(ds3231_class, NULL, myDeviceNr, NULL, DEVICE_NAME);
    if (ds3231_device == NULL) {
        class_destroy(ds3231_class);
        unregister_chrdev(myDeviceNr, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return (-1);
    }

    //initialize device file
    cdev_init(&my_device, &fops);

    if(cdev_add(&my_device, myDeviceNr, 1)== -1){
		printk("registering device fail\n");
		device_destroy(ds3231_class, myDeviceNr); return -1;
        return (-1);
    }

    //
    etx_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

	if(etx_i2c_adapter != NULL) {
		ds3231_client = i2c_acpi_new_device(ds3231_device,0, &ds3231_i2c_board_info);
		if(ds3231_client != NULL) {
			if(i2c_add_driver(&ds3231_driver) != -1) {
				ret = 0;
			}
			else
				printk("Can't add driver...\n");
		}
		i2c_put_adapter(etx_i2c_adapter);
	}
	printk("DS3231 Driver added!\n");

    //initialize sensor


    return ret;
}

static void __exit ds3231_exit(void)
{
    printk(KERN_INFO "Exiting ds3231 driver\n");
    i2c_del_driver(&ds3231_driver);

    printk("MyDeviceDriver - Goodbye, Kernel!\n");
	i2c_unregister_device(ds3231_client);
	i2c_del_driver(&ds3231_driver);
	cdev_del(&my_device);
    device_destroy(ds3231_class, myDeviceNr);
    class_destroy(ds3231_class);
    unregister_chrdev_region(myDeviceNr, 1);
}

module_init(ds3231_init);
module_exit(ds3231_exit);

MODULE_AUTHOR("Huy Tran-leader");
MODULE_DESCRIPTION("Driver for DS3231-I2C device");
MODULE_LICENSE("GPL");