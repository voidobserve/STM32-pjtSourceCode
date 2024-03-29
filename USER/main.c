/*************************************************
 Copyright ? 0010. All rights reserved.

// 文件名
@ File name: main.c -- 主函数所在位置
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:

            注意，不能使用Use MicroLIB来编译工程，否则测出的甲醛浓度和PM2.5浓度的数值会很大

@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"   // FreeRTOS任务
#include "semphr.h" // FreeRTOS互斥锁、信号量

#include "stm32f4xx.h"
#include "usart.h" // 串口1
#include "delay.h" // systick延时
#include "led.h"   // 开发板自带的四个LED
#include "beep.h"  // 开发板的蜂鸣器
#include "key.h"   // 开发板的按键
// #include "rtc.h"
#include "tim.h"

#include "dht11.h"             // 温湿度传感器
#include "ultraviolet_light.h" // 紫外线LED
#include "fan.h"               // 风扇
#include "ms1100.h"            // MS1100甲醛传感器
#include "dust_sensor.h"
#include "oled.h" // 0.96'OLED
#include "usart3.h"
#include "esp8266.h"
#include "mqtt.h"
#include "usart2.h" // 串口2初始化--蓝牙初始化

#include "interface.h"         // 各个界面
#include "child_safety_lock.h" // 儿童锁

TaskHandle_t Task_Initilize_Handle = NULL;   // 存放初始化任务的句柄
TaskHandle_t Task_ESP8266Init_Handle = NULL; // 存放ESP8266初始化以及连接阿里云任务的句柄
TaskHandle_t Task_Watch_Handle = NULL;       // 存放监视任务的句柄

TaskHandle_t Task_OLEDShow_Handle = NULL; // 存放OLED显示任务的句柄

TaskHandle_t Task_GetDeviceInfoPeriod_Handle = NULL; // 存放周期性获取设备信息任务的句柄
TaskHandle_t Task_MQTTPubilshPeriod_Handle = NULL;   // 存放MQTT周期性发布消息任务的句柄
TaskHandle_t Task_MQTTHeartBeatPacket_Handle = NULL; // 存放MQTT周期性发布心跳包任务的句柄

void Initialize_Task(void *arg);
void ESP8266_Connect_Task(void *arg);
void OLED_Show_Task(void *arg);
void MQTT_Pubilsh_Period_Task(void *arg);
void MQTT_Send_HeartBeatPacket_Task(void *arg);

void Watch_Task(void *arg);

// 硬件初始化任务，该任务只执行一次
void Initialize_Task(void *arg)
{
    // 中断优先级分组应该放在硬件初始化之前
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 设置系统中断优先级分组2
    // 1.硬件初始化
    uart_init(9600);
    printf("Hardware is Initing....\r\n");
    LED_Init();
    // KEY_Init();
    KEY_IT_Init();
    UlrtavioletLight_Init(); // 紫外线LED初始化
    FAN_PWM_Init();          // PWM风扇初始化
    DHT11_Init();            // 温湿度传感器初始化
    MS1100_Init();
    DustSensor_Init();
    USART3_Init(115200); // esp8266使用到的串口初始化
    USART2_Init(9600);   // 蓝牙初始化

    OLED_Init();
    TIM6_Init();

    OLED_Clear_Graphic_MEM();
    OLED_Clear(); // OLED清屏

    printf("Hardware Init is OK\r\n");

    vTaskDelete(NULL); // 初始化完成后，结束该任务
}

// ESP8266初始化以及连接阿里云物联网平台的任务，该任务只执行一次
void ESP8266_Connect_Task(void *arg)
{
    BaseType_t xReturned; // 保存创建任务函数的返回值

    printf("ESP8266 is Initing....\r\n");

    // 配置WiFi以STA联网模式工作
    while (Esp8266_Init())
        ;

    printf("ESP8266 Init is OK\r\n");

    // 连接阿里云服务器
    while (Mqtt_Connect_Aliyun())
        ; // 配置MQTT链接阿里云

    // 创建任务
    xReturned = xTaskCreate(MQTT_Pubilsh_Period_Task,     // 任务的函数指针
                            "MQTT_Pubilsh_Period_Task",   // 任务的名称
                            256,                          // 任务堆栈大小
                            NULL,                         // 不需要给任务传参，填NULL
                            3,                            // 任务的优先级
                            Task_MQTTPubilshPeriod_Handle // 任务的句柄
    );

    // 判断任务是否创建成功
    if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == xReturned)
    {
        printf("MQTT周期性发布消息任务创建失败\r\n");
    }

    // 创建任务
    xReturned = xTaskCreate(MQTT_Send_HeartBeatPacket_Task,   // 任务的函数指针
                            "MQTT_Send_HeartBeatPacket_Task", // 任务的名称
                            256,                              // 任务堆栈大小
                            NULL,                             // 不需要给任务传参，填NULL
                            3,                                // 任务的优先级
                            Task_MQTTHeartBeatPacket_Handle   // 任务的句柄
    );

    // 判断任务是否创建成功
    if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == xReturned)
    {
        printf("MQTT周期性发送心跳包任务创建失败\r\n");
    }

    vTaskDelete(NULL); // 初始化完成后，结束该任务
}

// 监视任务
void Watch_Task(void *arg)
{
    while (1)
    {
        vTaskSuspendAll();
        if (1 == USART2_RX_FALG)
        {

            printf("蓝牙收到了数据: %s \r\n", USART2_RX_BUF);

            if (strstr((const char *)USART2_RX_BUF, "UNLOCK"))
            {
                lock = false;
            }
            else if (strstr((const char *)USART2_RX_BUF, "LOCK"))
            {
                lock = true;
            }

            USART2_RX_FALG = 0;
        }

        mqtt_msg_handle();
        FAN_Speed_Adjust(); // 根据存放风扇转速的全局变量，调节风扇转速

        // 根据风扇是否转动，开关紫外线LED
        if (CUR_FAN_SPEED_CLOSE < cur_fan_speed)
        {
            ULTRAVIOLET_LIGHT_SET(0); // 紫外线LED亮
        }
        else
        {
            ULTRAVIOLET_LIGHT_SET(1); // 紫外线LED灭
        }

        xTaskResumeAll();

        vTaskDelay(500);
    }
}

// 通过MQTT周期性向阿里云物联网平台发送心跳包的任务
void MQTT_Send_HeartBeatPacket_Task(void *arg)
{
    while (1)
    {
        vTaskSuspendAll();
        // 每隔一段时间发送心跳包
        // 每6 * 5 s发送一次心跳包
        if (tim6_update_cnt >= 6)
        {
            /*	设备端在保活时间间隔内(保护时间在mqtt_connect设置为60s)，至少需要发送一次报文，包括ping请求。
                连接保活时间的取值范围为30秒~1200秒。建议取值300秒以上。
                从物联网平台发送CONNACK响应CONNECT消息时，开始心跳计时。收到PUBLISH、SUBSCRIBE、PING或 PUBACK消息时，会重置计时器。
            */
            // 发送心跳包，过于频繁发送心跳包，服务器将会持续一段时间不发送响应信息[可选]

            if (!mqtt_send_heart())
            {
            }
            else
            {
                // 如果断开阿里云，则重新配置MQTT链接阿里云
                while (Mqtt_Connect_Aliyun())
                    ;
            }

            tim6_update_cnt = 0; // 计数值清零
            printf("发送心跳包完成\r\n");
        }

        xTaskResumeAll();

        vTaskDelay(4000);
    }
}

// 周期性获取设备信息的任务
void Get_DeviceInfo_Task(void *arg)
{
    char *buf[50] = {0}; // 向串口1打印数据使用到的缓冲区

    while (1)
    {
        vTaskSuspendAll();
        DHT11_GetData(dhtbuf); // 获取温湿度数据
        sprintf((char *)buf, "温度 = %d ℃,湿度=%d %%RH \n\n", dhtbuf[2], dhtbuf[0]);
        printf("%s\r\n", (char *)buf);

        MS1100_Get_Mg_per_cubic_meter();

        DustSensor_GetVal();
        xTaskResumeAll();

        vTaskDelay(4000);
    }
}

// MQTT周期性发布消息的任务
void MQTT_Pubilsh_Period_Task(void *arg)
{
    while (1)
    {
        vTaskSuspendAll();

        // 利用MQTT上传给阿里云
        mqtt_report_devices_status();

        mqtt_report_device_hcho(hcho_mg_per_cubic_meter);

        mqtt_report_device_pm2_5(pm2_5_mg_per_cubic_meter);

        mqtt_report_device_fanspeed(cur_fan_speed);

        mqtt_report_device_lock(lock);

        xTaskResumeAll();

        vTaskDelay(2000);
    }
}

// OLED显示任务
void OLED_Show_Task(void *arg)
{
    while (1)
    {
        OLED_Show_Interface();
        vTaskDelay(200);
    }
}

// 程序入口
int main()
{

    BaseType_t xReturned; // 保存创建任务函数的返回值

    // 创建任务
    xReturned = xTaskCreate(Initialize_Task,      // 任务的函数指针
                            "Initialize_Task",    // 任务的名称
                            128,                  // 任务堆栈大小，128 * 32 / 8 == 512字节，32为CPU宽度，8代表8bit
                            NULL,                 // 不需要给任务传参，填NULL
                            4,                    // 任务的优先级
                            Task_Initilize_Handle // 任务的句柄
    );

    // 判断任务是否创建成功
    if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == xReturned)
    {
        printf("初始化任务创建失败\r\n");
    }

    // 创建任务
    xReturned = xTaskCreate(ESP8266_Connect_Task,   // 任务的函数指针
                            "ESP8266_Connect_Task", // 任务的名称
                            128,                    // 任务堆栈大小，128 * 32 / 8 == 512字节，32为CPU宽度，8代表8bit
                            NULL,                   // 不需要给任务传参，填NULL
                            3,                      // 任务的优先级
                            Task_ESP8266Init_Handle // 任务的句柄
    );

    // 判断任务是否创建成功
    if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == xReturned)
    {
        printf("ESP8266初始化任务创建失败\r\n");
    }

    // 创建任务
    xReturned = xTaskCreate(OLED_Show_Task,      // 任务的函数指针
                            "OLEDShowTask",      // 任务的名称
                            256,                 // 任务堆栈大小
                            NULL,                // 不需要给任务传参，填NULL
                            3,                   // 任务的优先级
                            Task_OLEDShow_Handle // 任务的句柄
    );

    // 判断任务是否创建成功
    if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == xReturned)
    {
        printf("OLED显示任务创建失败\r\n");
    }

    // 创建任务
    xReturned = xTaskCreate(Watch_Task,       // 任务的函数指针
                            "Watch_Task",     // 任务的名称
                            256,              // 任务堆栈大小
                            NULL,             // 不需要给任务传参，填NULL
                            3,                // 任务的优先级
                            Task_Watch_Handle // 任务的句柄
    );

    // 判断任务是否创建成功
    if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == xReturned)
    {
        printf("监视任务创建失败\r\n");
    }

    // 创建任务
    // 获取设备信息任务的堆栈空间要大一些
    xReturned = xTaskCreate(Get_DeviceInfo_Task,            // 任务的函数指针
                            "GetInfoTask",                  // 任务的名称
                            256,                            // 任务堆栈大小
                            NULL,                           // 不需要给任务传参，填NULL
                            3,                              // 任务的优先级
                            Task_GetDeviceInfoPeriod_Handle // 任务的句柄
    );

    // 判断任务是否创建成功
    if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == xReturned)
    {
        printf("周期性获取设备信息任务创建失败\r\n");
    }

    // 启动资源调度器
    vTaskStartScheduler();
}
