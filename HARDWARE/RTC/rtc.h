#ifndef __RTC_H
#define __RTC_H

#include "stm32f4xx.h"

extern volatile uint32_t uwTimeDisplay; // 表示RTC唤醒中断的标志

void My_RTC_Init(void);
void My_RTC_UpDate(void);
void My_RTC_ShowReflesh(void);
void My_RTC_AlarmInit(void);
void My_RTC_SetAlarm(void);


#endif
