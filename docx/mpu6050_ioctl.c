#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "mpu6050_driver"
#define CLASS_NAME "mpu6050"
#define DEVICE_NAME "mpu6050"

#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_PWR_MGMT_1 0x6B

// IOCTL commands
#define MPU6050_IOCTL_MAGIC 'm'
#define MPU6050_IOCTL_READ_X _IOR(MPU6050_IOCTL_MAGIC, 1, int)
#define MPU6050_IOCTL_READ_Y _IOR(MPU6050_IOCTL_MAGIC, 2, int)
#define MPU6050_IOCTL_READ_Z _IOR(MPU6050_IOCTL_MAGIC, 3, int)

static struct i2c_client *mpu6050_client;
static struct class* mpu6050_class = NULL;
static struct device* mpu6050_device = NULL;
static int major_number;

static int mpu6050_read_axis(struct i2c_client *client, int axis)
{
    u8 buf[6];
    s16 accel_data[3];

    if (i2c_smbus_read_i2c_block_data(client, MPU6050_REG_ACCEL_XOUT_H, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read accelerometer data\n");
        return -EIO;
    }

    // Combine high and low bytes to form 16-bit values
    accel_data[0] = (buf[0] << 8) | buf[1]; // X axis
    accel_data[1] = (buf[2] << 8) | buf[3]; // Y axis
    accel_data[2] = (buf[4] << 8) | buf[5]; // Z axis

    return accel_data[axis];
}

static long mpu6050_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int data;

    switch (cmd) {
        case MPU6050_IOCTL_READ_X:
            data = mpu6050_read_axis(mpu6050_client, 0);
            break;
        case MPU6050_IOCTL_READ_Y:
            data = mpu6050_read_axis(mpu6050_client, 1);
            break;
        case MPU6050_IOCTL_READ_Z:
            data = mpu6050_read_axis(mpu6050_client, 2);
            break;
        default:
            return -EINVAL;
    }

    if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
        return -EFAULT;
    }

    return 0;
}

static int mpu6050_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "MPU6050 device opened\n");
    return 0;
}

static int mpu6050_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "MPU6050 device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = mpu6050_open,
    .unlocked_ioctl = mpu6050_ioctl,
    .release = mpu6050_release,
};

static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    mpu6050_client = client;

    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

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

static void mpu6050_remove(struct i2c_client *client)
{
    device_destroy(mpu6050_class, MKDEV(major_number, 0));
    class_unregister(mpu6050_class);
    class_destroy(mpu6050_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "MPU6050 driver removed\n");
}

static const struct of_device_id mpu6050_of_match[] = {
    { .compatible = "invensense,mpu6050", },
    { },
};
MODULE_DEVICE_TABLE(of, mpu6050_of_match);

static struct i2c_driver mpu6050_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(mpu6050_of_match),
    },
    .probe      = mpu6050_probe,
    .remove     = mpu6050_remove,
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
MODULE_DESCRIPTION("MPU6050 I2C Client Driver with IOCTL Interface");
MODULE_LICENSE("GPL");
