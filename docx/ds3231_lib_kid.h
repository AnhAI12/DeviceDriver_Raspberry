#ifndef __DS3231_LIB_H__
#define __DS3231_LIB_H__

#define DS3231_DEV "/dev/ds3231"

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

int update_time(const char *path_to_dev);
int get_time(const char *path_to_dev, date_t *time);

#endif 
