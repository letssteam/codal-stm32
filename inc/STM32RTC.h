#pragma once

#if defined(STM32WBxx)
#include <cstdint>

#include "stm32_def.h"

enum class RTC_Hour_Format : uint32_t { HOUR_24 = RTC_HOURFORMAT_24, HOUR_12 = RTC_HOURFORMAT_12 };
enum class RTC_Time_AM_PM { AM = RTC_HOURFORMAT12_AM, PM = RTC_HOURFORMAT12_PM };
enum class RTC_Day_Saving {
    DISABLE = RTC_DAYLIGHTSAVING_NONE,
    ADD_1H  = RTC_DAYLIGHTSAVING_ADD1H,
    SUB_1H  = RTC_DAYLIGHTSAVING_SUB1H
};
enum class RTC_Output : uint32_t {
    DISABLE = RTC_OUTPUT_DISABLE,
    ALAMR_A = RTC_OUTPUT_ALARMA,
    ALARM_B = RTC_OUTPUT_ALARMB,
    WAKEUP  = RTC_OUTPUT_WAKEUP
};
enum class RTC_Output_Alarm_Remap : uint32_t { NONE = RTC_OUTPUT_REMAP_NONE, POS1 = RTC_OUTPUT_REMAP_POS1 };
enum class RTC_Output_Polarity : uint32_t { HIGH = RTC_OUTPUT_POLARITY_HIGH, LOW = RTC_OUTPUT_POLARITY_LOW };
enum class RTC_Output_Type : uint32_t { OPENDRAIN = RTC_OUTPUT_TYPE_OPENDRAIN, PUSHPULL = RTC_OUTPUT_TYPE_PUSHPULL };

enum class RTC_Week_Day : uint8_t {
    MONDAY    = RTC_WEEKDAY_MONDAY,
    TUESDAY   = RTC_WEEKDAY_TUESDAY,
    WEDNESDAY = RTC_WEEKDAY_WEDNESDAY,
    THURSDAY  = RTC_WEEKDAY_THURSDAY,
    FRIDAY    = RTC_WEEKDAY_FRIDAY,
    SATURDAY  = RTC_WEEKDAY_SATURDAY,
    SUNDAY    = RTC_WEEKDAY_SUNDAY,
};

enum class RTC_Month {
    JANUARY   = RTC_MONTH_JANUARY,
    FEBRUARY  = RTC_MONTH_FEBRUARY,
    MARCH     = RTC_MONTH_MARCH,
    APRIL     = RTC_MONTH_APRIL,
    MAY       = RTC_MONTH_MAY,
    JUNE      = RTC_MONTH_JUNE,
    JULY      = RTC_MONTH_JULY,
    AUGUST    = RTC_MONTH_AUGUST,
    SEPTEMBER = RTC_MONTH_SEPTEMBER,
    OCTOBER   = RTC_MONTH_OCTOBER,
    NOVEMBER  = RTC_MONTH_NOVEMBER,
    DECEMBER  = RTC_MONTH_DECEMBER,
};

struct RTC_Time {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    RTC_Time_AM_PM amPm;
};

struct RTC_Date {
    RTC_Week_Day weekday;
    uint8_t day;
    RTC_Month month;
    uint8_t year;
};

namespace codal {
class STM32RTC {
  public:
    STM32RTC();
    ~STM32RTC();

    /**
     * @brief Initialize de RTC
     *
     * @param hourFormat Set the format to 24h or 12h
     * @param output Set the RTC ouput
     * @param alarmRemap Set the pin remap
     * @param outPolarity Set the output polarity
     * @param outType Set the output type
     * @return true if successful, false otherwise
     */
    bool init(RTC_Hour_Format hourFormat = RTC_Hour_Format::HOUR_24, RTC_Output output = RTC_Output::DISABLE,
              RTC_Output_Alarm_Remap alarmRemap = RTC_Output_Alarm_Remap::NONE,
              RTC_Output_Polarity outPolarity   = RTC_Output_Polarity::HIGH,
              RTC_Output_Type outType           = RTC_Output_Type::OPENDRAIN);

    /**
     * @brief Set the Daylight Saving time parameter.
     *
     * @param saving Set the daylight saving time to enable
     * @return true if successful, false otherwise
     */
    bool setDaylightSaving(RTC_Day_Saving saving);

    /**
     * @brief To find out whether a Daylight Saving is active
     *
     * @return true is enable, false otherwise
     */
    bool isDaylightSavingEnable();

    /**
     * @brief Set the Time
     *
     * @param hours the hour value (in selected format 12h/24h) [0..23]
     * @param minutes the minutes value [0..59]
     * @param seconds the seconds value [0..59]
     * @param am_pm AM or PM (only for 12h format)
     * @return true if successful, false otherwise
     */
    bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds, RTC_Time_AM_PM am_pm = RTC_Time_AM_PM::AM);

    /**
     * @brief Get the Time from RTC
     *
     * @return RTC_Time
     */
    RTC_Time getTime();

    /**
     * @brief Set the Date
     *
     * @param weekday the name of the day
     * @param day the day value [1..31]
     * @param month the month value
     * @param year the year value on 2 digit [0..99]
     * @return true if successful, false otherwise
     */
    bool setDate(RTC_Week_Day weekday, uint8_t day, RTC_Month month, uint8_t year);

    /**
     * @brief Get the Date from RTC
     *
     * @return RTC_Date
     */
    RTC_Date getDate();

    /**
     * @returns true if the time format is 12h, false otherwise
     */
    bool isHourFormat12() { return isHourMode12; }

    /**
     * @returns true if the time format is 24h, false otherwise
     */
    bool isHourFormat24() { return !isHourMode12; }

  private:
    RTC_HandleTypeDef* hrtc;
    bool isHourMode12;
    RTC_Day_Saving daySaving;

    void configureClock();
};
}  // namespace codal
#else
#warning "STM32RTC is not supported by your target !"
#endif