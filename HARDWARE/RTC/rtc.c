#include "rtc.h"
#include "stm32f4xx.h"
#include <string.h>

#include "usart.h"

volatile uint32_t uwTimeDisplay = 0; // 表示RTC唤醒中断的标志

void My_RTC_Init(void)
{
    RTC_InitTypeDef RTC_InitStructure;
    RTC_DateTypeDef RTC_DateStructure;
    RTC_TimeTypeDef RTC_TimeStructure;

    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    // 1.打开PWR外设时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    // 2.使能RTC寄存器的写访问
    PWR_BackupAccessCmd(ENABLE);

    // 3.使能LSE低速外部时钟 32.768KHZ  精度高  功耗低
    RCC_LSEConfig(RCC_LSE_ON);

    // 4.等待LSE低速外部时钟就绪
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        ;

    // 5.选择LSE作为RTC外设的时钟源  可以分频得到准确的1HZ
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    // 6.打开RTC外设的时钟
    RCC_RTCCLKCmd(ENABLE);

    // 7.等待RTC寄存器同步完成
    RTC_WaitForSynchro();

    // 8.对RTC进行时钟分频并初始化  32768HZ / (127+1) * (255+1) = 1HZ
    RTC_InitStructure.RTC_AsynchPrediv = 128 - 1;
    RTC_InitStructure.RTC_SynchPrediv = 256 - 1;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; // 24小时制
    RTC_Init(&RTC_InitStructure);

    // 9.设置RTC的日期  2024/2/26
    RTC_DateStructure.RTC_Year = 0x24;    // 年份
    RTC_DateStructure.RTC_Month = 0x02;   // 月份
    RTC_DateStructure.RTC_Date = 0x26;    // 日期
    RTC_DateStructure.RTC_WeekDay = 0x01; // 星期
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

    // 10.设置RTC的时间 15:51:30
    RTC_TimeStructure.RTC_H12 = RTC_H12_PM; // 下午
    RTC_TimeStructure.RTC_Hours = 0x15;     // 时钟
    RTC_TimeStructure.RTC_Minutes = 0x51;   // 分钟
    RTC_TimeStructure.RTC_Seconds = 0x30;   // 秒钟
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

    // 11.设置唤醒中断参数  NVIC的参数
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 12.配置EXTI22外部中断 连接到RTC唤醒中断
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 13.清除RTC的唤醒中断的标志
    RTC_ClearITPendingBit(RTC_IT_WUT);

    // 14.关闭RTC唤醒定时器
    RTC_WakeUpCmd(DISABLE);

    // 15.选择RTC唤醒时钟源  选择内部时钟(1HZ)
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);

    // 16.设置唤醒计数器的值 设置为0，表示计数一次就触发唤醒
    RTC_SetWakeUpCounter(0);

    // 17.选择唤醒中断  选择中断源
    RTC_ITConfig(RTC_IT_WUT, ENABLE);

    // 18.打开唤醒定时器
    RTC_WakeUpCmd(ENABLE);
}

// 通过检测串口发送过来的消息，更新RTC内的时间/日期信息
// settime-hh-mm-ss
// setdate-yy-mm-dd-weekday
void My_RTC_UpDate(void)
{
    RTC_DateTypeDef RTC_DateStructure;
    RTC_TimeTypeDef RTC_TimeStructure;
    u8 temp = 0;

    // settime-hh-mm-ss
    if (strncmp((const char *)USART1_RX_BUF, "settime", strlen("settime")) == 0)
    {
        // 如果收到了要设置时间的命令
        temp = (USART1_RX_BUF[8] - '0') * 10;
        temp += USART1_RX_BUF[9] - '0';
        if (temp > 12)
        {
            // 如果小时部分的时间大于12，说明是下午
            RTC_TimeStructure.RTC_H12 = RTC_H12_PM;
        }
        else
        {
            RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
        }

        RTC_TimeStructure.RTC_Hours = (temp / 10) * 16 + (temp % 10);

        temp = (USART1_RX_BUF[11] - '0') * 10;
        temp += USART1_RX_BUF[12] - '0';
        RTC_TimeStructure.RTC_Minutes = (temp / 10) * 16 + (temp % 10);

        temp = (USART1_RX_BUF[14] - '0') * 10;
        temp += USART1_RX_BUF[15] - '0';
        RTC_TimeStructure.RTC_Seconds = (temp / 10) * 16 + (temp % 10);

        RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

        // memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF) / sizeof(USART_RX_BUF[0]));
    }
    else if (strncmp((const char *)USART1_RX_BUF, "setdate", strlen("setdate")) == 0)
    {
        // setdate-yy-mm-dd-weekday
        // 如果收到了要设置时间的命令
        temp = (USART1_RX_BUF[8] - '0') * 10;
        temp += USART1_RX_BUF[9] - '0';
        RTC_DateStructure.RTC_Year = (temp / 10) * 16 + (temp % 10);

        temp = (USART1_RX_BUF[11] - '0') * 10;
        temp += USART1_RX_BUF[12] - '0';
        RTC_DateStructure.RTC_Month = (temp / 10) * 16 + (temp % 10);

        temp = (USART1_RX_BUF[14] - '0') * 10;
        temp += USART1_RX_BUF[15] - '0';
        RTC_DateStructure.RTC_Date = (temp / 10) * 16 + (temp % 10);

        temp = USART1_RX_BUF[17] - '0';
        RTC_DateStructure.RTC_WeekDay = temp;
        RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

        // memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF) / sizeof(USART_RX_BUF[0]));
    }
}

// 将RTC的时间和日期信息更新到串口1
void My_RTC_ShowReflesh(void)
{
    RTC_DateTypeDef RTC_DateStructure;
    RTC_TimeTypeDef RTC_TimeStructure;
    uint8_t aShowTime[50] = {0}; // 用于存储RTC的系统时间

    // 获取RTC时间
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

    // 对RTC时间进行转换
    sprintf((char *)aShowTime, "时间：%0.2d:%0.2d:%0.2d", RTC_TimeStructure.RTC_Hours,
            RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    printf("%s\r\n", aShowTime);
    memset(aShowTime, 0, sizeof(aShowTime) / sizeof(aShowTime[0]));

    sprintf((char *)aShowTime, "日期：20%0.2d-%0.2d-%0.2d-星期%0.1d", RTC_DateStructure.RTC_Year,
            RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date, RTC_DateStructure.RTC_WeekDay);
    printf("%s\r\n", aShowTime);
    // memset(aShowTime, 0, sizeof(aShowTime) / sizeof(aShowTime[0]));

    // memset(&RTC_TimeStructure, 0, sizeof(RTC_TimeStructure));
    // memset(&RTC_DateStructure, 0, sizeof(RTC_DateStructure));
}

// RTC闹钟配置
void My_RTC_AlarmInit(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    RTC_AlarmTypeDef RTC_AlarmStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1.打开PWR外设时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    // 2.使能RTC寄存器的写访问
    PWR_BackupAccessCmd(ENABLE);

    // 3.使能LSE低速外部时钟 32.768KHZ  精度高  功耗低
    RCC_LSEConfig(RCC_LSE_ON);

    // 4.等待LSE低速外部时钟就绪
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        ;

    // 5.选择LSE作为RTC外设的时钟源  可以分频得到准确的1HZ
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    // 6.打开RTC外设的时钟
    RCC_RTCCLKCmd(ENABLE);

    // 7.等待RTC寄存器同步完成
    RTC_WaitForSynchro();

    // 配置外部中断线
    EXTI_ClearITPendingBit(EXTI_Line17); // 清除RTC闹钟的中断标志
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 使能RTC闹钟中断
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    RTC_ClearFlag(RTC_FLAG_ALRAF); // 清除闹钟A的标志

    // 设置闹钟A的掩码（触发闹钟的参数）
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;                     // 上午
    RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;              // 闹钟的触发和日期无关，即每天都会触发闹钟
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date; // 闹钟触发与具体的那一天有关
    RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x01;                            // ？？
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = 0x00;                         // 小时
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x00;                       // 分钟
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x00;                       // 秒
    RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

    /* Set alarm A sub seconds and enable SubSec Alarm : generate 8 interrupts per Second */
    // 设置闹钟A的亚秒，并使能亚秒闹钟
    // RTC_AlarmSubSecondConfig(RTC_Alarm_A, 0xFF, RTC_AlarmSubSecondMask_SS14_5);

    // 使能闹钟A的中断
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);

    // 清除闹钟A的中断标志
    RTC_ClearITPendingBit(RTC_IT_ALRA);

    // 打开闹钟A
    RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}

// 通过接收串口发送的命令来设置闹钟
// setalarm-hh-mm-ss
void My_RTC_SetAlarm(void)
{
    RTC_AlarmTypeDef RTC_AlarmStructure;
    u8 temp = 0;

    // setalarm-hh-mm-ss
    if (strncmp((const char *)USART1_RX_BUF, "setalarm", strlen("setalarm")) == 0)
    {
        // 先解除RTC寄存器的写保护

        // 1.打开PWR外设时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

        // 2.使能RTC寄存器的写访问
        PWR_BackupAccessCmd(ENABLE);

        // 3.使能LSE低速外部时钟 32.768KHZ  精度高  功耗低
        RCC_LSEConfig(RCC_LSE_ON);

        // 4.等待LSE低速外部时钟就绪
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
            ;

        // 5.选择LSE作为RTC外设的时钟源  可以分频得到准确的1HZ
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

        // 6.打开RTC外设的时钟
        RCC_RTCCLKCmd(ENABLE);

        // 7.等待RTC寄存器同步完成
        RTC_WaitForSynchro();

        RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

        RTC_ClearFlag(RTC_FLAG_ALRAF); // 清除闹钟A的标志

        // 如果收到了要设置时间的命令
        temp = (USART1_RX_BUF[9] - '0') * 10;
        temp += USART1_RX_BUF[10] - '0';

        if (temp > 12)
        {
            RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;
        }
        else
        {
            RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
        }

        RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = (temp / 10) * 16 + (temp % 10);

        temp = (USART1_RX_BUF[12] - '0') * 10;
        temp += USART1_RX_BUF[13] - '0';
        RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = (temp / 10) * 16 + (temp % 10);

        temp = (USART1_RX_BUF[15] - '0') * 10;
        temp += USART1_RX_BUF[16] - '0';
        RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = (temp / 10) * 16 + (temp % 10);

        RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_All;
        RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
        RTC_AlarmStructure.RTC_AlarmDateWeekDay = RTC_Weekday_Monday;
        RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;                     // 上午
        RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;              // 闹钟的触发和日期无关，即每天都会触发闹钟
        RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date; // 闹钟触发与具体的那一天有关
        RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

        // 清除闹钟A的中断标志
        RTC_ClearITPendingBit(RTC_IT_ALRA);

        RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

        printf("set alarm success\r\n");
    }
}
