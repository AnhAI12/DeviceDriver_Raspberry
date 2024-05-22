/*
	
	
	Function: set_time,
				read_time (optional mode read)

	Preference: SMBus, I2C
		https://www.kernel.org/doc/html/v5.5/i2c/smbus-protocol.html
		https://www.kernel.org/doc/html/latest/i2c/writing-clients.html
*/



#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#define DRIVER_NAME "ds3231_driver"

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

static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;

    // Set power management register to wake up device
    ret = i2c_smbus_write_byte_data(client, MPU6050_REG_PWR_MGMT_1, 0);
    if (ret < 0) {
        printk(KERN_ERR "Failed to wake up MPU6050\n");
        return ret;
    }

    mpu6050_client = client;

    // Read data from MPU6050 sensor
    ret = mpu6050_read_data(client);
    if (ret < 0) {
        return ret;
    }

    printk(KERN_INFO "MPU6050 driver installed\n");

    return 0;
}

static void mpu6050_remove(struct i2c_client *client)
{
    printk(KERN_INFO "MPU6050 driver removed\n");

    // Clean up
 
}

static const struct i2c_device_id mpu6050_id[] = {
    { "mpu6050", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, mpu6050_id);

static struct i2c_driver mpu6050_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
    },
    .probe      = mpu6050_probe,
    .remove     = mpu6050_remove,
    .id_table   = mpu6050_id,
};

static int __init mpu6050_init(void)
{
    printk(KERN_INFO "Initializing MPU6050 driver\n");
    return i2c_add_driver(&mpu6050_driver);
}

static void __exit mpu6050_exit(void)
{
    printk(KERN_INFO "Exiting MPU6050 driver\n");
    i2c_del_driver(&mpu6050_driver);
}

module_init(mpu6050_init);
module_exit(mpu6050_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("MPU6050 I2C Client Driver");
MODULE_LICENSE("GPL");