#ifndef __DS3231_H__
#define __DS3231_H__

#define DS3231_REG_SECOND 	0x00 /* 00 - 59 */
#define DS3231_REG_MINUTE 	0x01 /* 00 - 59 */
#define DS3231_REG_HOUR  	0x02 /* 00 - 23 */
#define DS3231_REG_DAY    	0x03 /* 01 - 07 */

#define DS3231_REG_DATE   	0x04 
#define DS3231_REG_MONTH  	0x05
#define DS3231_REG_YEAR   	0x06

#define DS3231_A1_SECOND	0x07
#define DS3231_A1_MINUTE	0x08
#define DS3231_A1_HOUR		0x09
#define DS3231_A1_DATE		0x0a

#define DS3231_A2_MINUTE	0x0b
#define DS3231_A2_HOUR		0x0c
#define DS3231_A2_DATE		0x0d

#define DS3231_REG_CONTROL  0x0e
#define DS3231_REG_STATUS   0x0f

typedef unsigned char u8;
typedef unsigned short u16;

typedef struct {
    u8 date;
    u8 month;
    u16 year;
    u8 hour;
    u8 min;
    u8 sec;
    char day[10];
} date_t;

const char day_of_weak[7][9] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};
typedef struct {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} tm_usr_t;
#define UPDATE_TIME _IOW('U', 1, struct tm *)
#define GET_TIME _IOR('G', 1, date_t *)

#endif /* DS3231 */
