// timekeeping registers
#define DS3231_SEC_ADDR         0x00
#define DS3231_MIN_ADDR         0x01
#define DS3231_HOUR_ADDR        0x02
#define DS3231_DAY_ADDR         0x03
#define DS3231_DATE_ADDR        0x04
#define DS3231_MONTH_ADDR       0x05
#define DS3231_YEAR_ADDR        0x06


typedef struct 
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_day;
    int tm_month;
    long tm_year;
    int tm_wday;
    int tm_yday;
}tm;
