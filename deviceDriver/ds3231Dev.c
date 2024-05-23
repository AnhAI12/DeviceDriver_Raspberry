/*
	
	
	Function: set_time,
				read_time (optional mode read)

	Preference: SMBus, I2C
		https://www.kernel.org/doc/html/v5.5/i2c/smbus-protocol.html
		https://www.kernel.org/doc/html/latest/i2c/writing-clients.html
		
	Function:
		i2c_smbus_write_block_data(struct i2c_client *client,
                               u8 command, u8 length, const u8 *values);
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

#define DRIVER_NAME "ds3231_driver"
#define CLASS_NAME "mpu6050"
#define DEVICE_NAME "mpu6050"

#define MPU6050_ADDR 0x68
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_PWR_MGMT_1 0x6B

static struct i2c_client *mpu6050_client;

static int mpu6050_read_data(struct i2c_client *client)
{
    u8 buf[6];
    s16 accel_x, accel_y, accel_z;

    // Read accelerometer data
    if (i2c_smbus_read_i2c_block_data(client, MPU6050_REG_ACCEL_XOUT_H, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read accelerometer data\n");
        return -EIO;
    }

    // Combine high and low bytes to form 16-bit values
    accel_x = (buf[0] << 8) | buf[1];
    accel_y = (buf[2] << 8) | buf[3];
    accel_z = (buf[4] << 8) | buf[5];

    // Print accelerometer data
    printk(KERN_INFO "Accelerometer: X=%d, Y=%d, Z=%d\n", accel_x, accel_y, accel_z);

    return 0;
}

static struct file_operations fops = {
    .open = ds3231_open,
    .unlocked_ioctl = ds3231_ioctl,
    .release = ds3231_release,
};

static int ds3231_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	ds3231_client = client;

    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }
	printk("My driver major = %d \n", major_number);

    mpu6050_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mpu6050_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(mpu6050_class);
    }

    mpu6050_device = device_create(mpu6050_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mpu6050_device)) {
        class_destroy(mpu6050_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(mpu6050_device);
    }

    printk(KERN_INFO "MPU6050 driver installed\n");

    return 0;
}

static void ds3231_remove(struct i2c_client *client)
{
    printk(KERN_INFO "MPU6050 driver removed\n");

    // Clean up
 
}

//	Struct of I2C device driver

static const struct i2c_device_id ds3231_id[] = {
    { "ds3231", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, ds3231_id);

static struct i2c_driver ds3231_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
    },
    .probe      = ds3231_probe,	//bind to device
    .remove     = ds3231_remove,
    .id_table   = ds3231_id,
};

static int __init ds3231_init(void)
{
    printk(KERN_INFO "Initializing DS3231 driver\n");
    return i2c_add_driver(&ds3231_driver);
}

static void __exit ds3231_exit(void)
{
    printk(KERN_INFO "Exiting MPU6050 driver\n");
    i2c_del_driver(&ds3231_driver);
}

module_init(ds3231_init);
module_exit(ds3231_exit);

MODULE_AUTHOR("Huy Tran-leader");
MODULE_DESCRIPTION("Driver for DS3231-I2C device");
MODULE_LICENSE("GPL");