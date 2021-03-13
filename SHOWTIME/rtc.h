#ifndef __RTC_H
#define __RTC_H

#include "stm32f4xx.h"


void auto_set_Alarm();
void Rtc_Init(void);
void RTC_A_Alarm(void);
void change(void);
void Alarm_time();
void stop_start_Alarm();

#endif