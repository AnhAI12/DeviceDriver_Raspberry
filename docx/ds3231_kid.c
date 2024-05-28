#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include "ds3231.h"
struct ds3231 {
    struct i2c_client *client;
    struct miscdevice myRtc;
};

static u8 ds3231_write8(struct ds3231 *rtc, u8 reg, u8 value);
static u8 ds3231_write16(struct ds3231 *rtc, u8 reg, u16 value);
static u8 ds3231_read8(struct ds3231 *rtc, u8 reg);
static u16 ds3231_read16(struct ds3231 *rtc, u8 reg);
static void ds3231_update_real_time(struct ds3231 *rtc, tm_usr_t time_info);
static void ds3231_init(struct ds3231 *rtc);
static u8 encodeBCD8(u8 value);
static u16 encodeBCD16(u16 value);
static u8 decodeBCD8(u8 value);
static u16 decodeBCD16(u16 value);
static u8 ds3231_read_time(struct ds3231 *rtc, date_t *time);
static int ds3231_open(struct inode *inodep, struct file *filep);
static int ds3231_close(struct inode *inodep, struct file *filep);
static long ds3231_ioctl (struct file *filep, unsigned int cmd, unsigned long data);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ds3231_open,
    .release = ds3231_close,
    .unlocked_ioctl = ds3231_ioctl,
};

static int ds3231_probe(struct i2c_client *client) {
    struct ds3231 *rtc;
    rtc = devm_kzalloc(&client->dev, sizeof(*rtc), GFP_KERNEL);
    if (!rtc)
        return -ENOMEM;
    i2c_set_clientdata(client, rtc);
    rtc->client = client;
    /* miscdevice register */
    rtc->myRtc.minor = MISC_DYNAMIC_MINOR; // dynamic allocator minor number
    rtc->myRtc.name = "ds3231";            // name of device file
    rtc->myRtc.fops = &fops;
    rtc->myRtc.mode = 0666;
    ds3231_init(rtc);
    return misc_register(&rtc->myRtc);
}
static void ds3231_remove(struct i2c_client *client) {
    struct ds3231 *rtc = i2c_get_clientdata(client);
    if (rtc)
        misc_deregister(&rtc->myRtc);
}
static const struct i2c_device_id ds3231_id_table[] = {
    { "rtc-ds3231", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ds3231_id_table);
static const struct of_device_id ds3231_of_match[] = {
    { .compatible = "rtc-ds3231", },
    { }
};
MODULE_DEVICE_TABLE(of, ds3231_of_match);
static struct i2c_driver ds3231_drv = {
    .probe = ds3231_probe,
    .remove = ds3231_remove,
    .driver = {
        .name = "rtc-ds3231",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(ds3231_of_match),
    },
    .id_table = ds3231_id_table,
};
static u8 encodeBCD8(u8 value) {
    return (value % 10 + ((value / 10) << 4));
}
static u16 encodeBCD16(u16 value) {
    return ((value % 10) + (((value / 10) % 10) << 4) + (((value / 100) % 10) << 8) + (((value / 1000) % 10) << 12));
}
static u8 decodeBCD8(u8 value) {
    return (((value & 0xf0) >> 4) * 10) + (value & 0x0f);
}
static u16 decodeBCD16(u16 value) {
    return (((value & 0xf000) >> 12) * 1000) + (((value & 0x0f00) >> 8) * 100) + (((value & 0xf0) >> 4) * 10) + (value & 0x0f);
}
static u8 ds3231_write8(struct ds3231 *rtc, u8 reg, u8 value) {
     return i2c_smbus_write_byte_data(rtc->client, reg, value);
}
static u8 ds3231_write16(struct ds3231 *rtc, u8 reg, u16 value) {
     return i2c_smbus_write_word_data(rtc->client, reg, value);
}
static u8 ds3231_read8(struct ds3231 *rtc, u8 reg) {
     return i2c_smbus_read_byte_data(rtc->client, reg);
}
static u16 ds3231_read16(struct ds3231 *rtc, u8 reg) {
     return i2c_smbus_read_word_data(rtc->client, reg);
}
static void ds3231_update_real_time(struct ds3231 *rtc, tm_usr_t time_info) {
    ds3231_write8(rtc, DS3231_REG_DAY, encodeBCD8(time_info.tm_wday));
    ds3231_write8(rtc, DS3231_REG_DATE, encodeBCD8(time_info.tm_mday));
    ds3231_write8(rtc, DS3231_REG_MONTH, encodeBCD8(time_info.tm_mon + 1));
    ds3231_write16(rtc, DS3231_REG_YEAR, encodeBCD16(time_info.tm_year + 1900));
    ds3231_write8(rtc, DS3231_REG_HOUR, encodeBCD8(time_info.tm_hour));
    ds3231_write8(rtc, DS3231_REG_MINUTE, encodeBCD8(time_info.tm_min));
    ds3231_write8(rtc, DS3231_REG_SECOND, encodeBCD8(time_info.tm_sec));
}
static void ds3231_init(struct ds3231 *rtc) {
    u8 foo;
    foo = ds3231_read8(rtc, DS3231_REG_CONTROL);
    foo &= ~0x03; // disable alarm
    ds3231_write8(rtc, DS3231_REG_CONTROL, foo);
    ds3231_write8(rtc, DS3231_REG_STATUS, 0x00); // clear all flag
}
static u8 ds3231_read_time(struct ds3231 *rtc, date_t *time) {
    u8 day;
    time->hour = decodeBCD8(ds3231_read8(rtc, DS3231_REG_HOUR));
    time->min = decodeBCD8(ds3231_read8(rtc, DS3231_REG_MINUTE));
    time->sec = decodeBCD8(ds3231_read8(rtc, DS3231_REG_SECOND));

    time->date = decodeBCD8(ds3231_read8(rtc, DS3231_REG_DATE));
    time->month = decodeBCD8(ds3231_read8(rtc, DS3231_REG_MONTH));
    time->year = decodeBCD16(ds3231_read16(rtc, DS3231_REG_YEAR));
    day = decodeBCD8(ds3231_read8(rtc, DS3231_REG_DAY));
    memset(time->day, 0, 10);
    memcpy(time->day, day_of_weak[day], strlen(day_of_weak[day]));
    return 0;
}

static int ds3231_open(struct inode *inodep, struct file *filep) {
    struct ds3231 *rtc = NULL;
    rtc = container_of(filep->private_data, struct ds3231, myRtc);
    if (rtc) {
        filep->private_data = rtc;
        return 0;
    } else
        return -EFAULT;
}
static int ds3231_close(struct inode *inodep, struct file *filep) {
    filep->private_data = NULL;
    return 0;
}
static long ds3231_ioctl (struct file *filep, unsigned int cmd, unsigned long data) {
    long res;
    struct ds3231 *rtc = filep->private_data;
    date_t myTime;
    tm_usr_t realTime;
    if (!rtc)
        return -EFAULT;
    switch (cmd) {
        case UPDATE_TIME:
            res = copy_from_user(&realTime, (tm_usr_t *)data, sizeof(tm_usr_t));
            ds3231_update_real_time(rtc, realTime);
            break;
        case GET_TIME:
            ds3231_read_time(rtc, &myTime);
            res = copy_to_user((date_t *)data, &myTime, sizeof(date_t));
            break;
        default:
            break;
    }
    return res;
}

module_i2c_driver(ds3231_drv);

module_init(ds3231_init);
module_exit(ds3231_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DinhNam <20021163@vnu.edu.vn>");
MODULE_DESCRIPTION("I2C Regmap Test for DS3231");
MODULE_VERSION("1.0");
