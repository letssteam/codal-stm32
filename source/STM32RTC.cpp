#include "STM32RTC.h"
#if defined(STM32WBxx)
using namespace codal;

STM32RTC::STM32RTC() : hrtc{new RTC_HandleTypeDef()}, isHourMode12{true}, daySaving{RTC_Day_Saving::DISABLE} {}
STM32RTC::~STM32RTC() {}

bool STM32RTC::init(RTC_Hour_Format hourFormat, RTC_Output output, RTC_Output_Alarm_Remap alarmRemap,
                    RTC_Output_Polarity outPolarity, RTC_Output_Type outType)
{
    RTC_InitTypeDef init;
    init.HourFormat     = static_cast<uint32_t>(hourFormat);
    init.AsynchPrediv   = 127;  // Default value for a RTC clock to 32kHz
    init.SynchPrediv    = 255;  // Default value for a RTC clock to 32kHz
    init.OutPut         = static_cast<uint32_t>(output);
    init.OutPutRemap    = static_cast<uint32_t>(alarmRemap);
    init.OutPutPolarity = static_cast<uint32_t>(outPolarity);
    init.OutPutType     = static_cast<uint32_t>(outType);

    hrtc->Instance = RTC;
    hrtc->Init     = init;

    isHourMode12 = (hourFormat == RTC_Hour_Format::HOUR_12);

    configureClock();
    return HAL_RTC_Init(hrtc) == HAL_OK;
}

bool STM32RTC::setDaylightSaving(RTC_Day_Saving saving)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef _date;
    HAL_RTC_GetTime(hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(hrtc, &_date, RTC_FORMAT_BIN);

    daySaving = saving;

    time.DayLightSaving = static_cast<uint32_t>(daySaving);
    time.StoreOperation = (daySaving != RTC_Day_Saving::DISABLE) ? RTC_STOREOPERATION_SET : RTC_STOREOPERATION_RESET;

    return HAL_RTC_SetTime(hrtc, &time, RTC_FORMAT_BIN) == HAL_OK;
}

bool STM32RTC::isDaylightSavingEnable()
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef _date;
    HAL_RTC_GetTime(hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(hrtc, &_date, RTC_FORMAT_BIN);

    return (time.StoreOperation == RTC_STOREOPERATION_SET);
}

bool STM32RTC::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds, RTC_Time_AM_PM am_pm)
{
    if (hours > 23 || minutes > 59 || seconds > 59) {
        return false;
    }

    if (isHourMode12 && hours > 12) {
        return false;
    }

    RTC_TimeTypeDef time;
    time.Hours          = hours;
    time.Minutes        = minutes;
    time.Seconds        = seconds;
    time.TimeFormat     = static_cast<uint8_t>(am_pm);
    time.DayLightSaving = static_cast<uint32_t>(daySaving);
    time.StoreOperation = (daySaving != RTC_Day_Saving::DISABLE) ? RTC_STOREOPERATION_SET : RTC_STOREOPERATION_RESET;

    return HAL_RTC_SetTime(hrtc, &time, RTC_FORMAT_BIN) == HAL_OK;
}

RTC_Time STM32RTC::getTime()
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef _date;
    HAL_RTC_GetTime(hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(hrtc, &_date, RTC_FORMAT_BIN);

    return RTC_Time{.hours   = time.Hours,
                    .minutes = time.Minutes,
                    .seconds = time.Seconds,
                    .amPm    = static_cast<RTC_Time_AM_PM>(time.TimeFormat)};
}

bool STM32RTC::setDate(RTC_Week_Day weekday, uint8_t day, RTC_Month month, uint8_t year)
{
    if (day == 0 || day > 31 || year > 99) {
        return false;
    }

    RTC_DateTypeDef date;

    date.WeekDay = static_cast<uint8_t>(weekday);
    date.Date    = day;
    date.Month   = static_cast<uint8_t>(month);
    date.Year    = year;

    return HAL_RTC_SetDate(hrtc, &date, RTC_FORMAT_BIN) == HAL_OK;
}

RTC_Date STM32RTC::getDate()
{
    RTC_TimeTypeDef _time;
    RTC_DateTypeDef date;
    HAL_RTC_GetTime(hrtc, &_time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(hrtc, &date, RTC_FORMAT_BIN);

    return RTC_Date{.weekday = static_cast<RTC_Week_Day>(date.WeekDay),
                    .day     = date.Date,
                    .month   = static_cast<RTC_Month>(date.Month),
                    .year    = date.Year};
}

void STM32RTC::configureClock()
{
    RCC_PeriphCLKInitTypeDef rtcClk;

    rtcClk.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    rtcClk.RTCClockSelection    = RCC_RTCCLKSOURCE_LSE;

    HAL_RCCEx_PeriphCLKConfig(&rtcClk);
    __HAL_RCC_RTC_ENABLE();
}
#endif