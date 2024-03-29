#ifndef __MQTT_H
#define __MQTT_H

//包含头文件
#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp8266.h"

//宏定义
//此处是阿里云服务器的公共实例登陆配置------------注意修改为自己的云服务设备信息！！！！
#define MQTT_BROKERADDRESS		    "iot-060aayl4.mqtt.iothub.aliyuncs.com"
#define MQTT_CLIENTID				"k0w9gEec09K.air_cleaner|securemode=2,signmethod=hmacsha256,timestamp=1709975638990|"
#define MQTT_USARNAME				"air_cleaner&k0w9gEec09K"
#define MQTT_PASSWD					"7dd886579fcea81e460411ccef09cca9f3fd9a7eb1a6a16d026d678a3633f99d"
#define	MQTT_PUBLISH_TOPIC		    "/sys/k0w9gEec09K/air_cleaner/thing/event/property/post"
#define MQTT_SUBSCRIBE_TOPIC	    "/sys/k0w9gEec09K/air_cleaner/thing/service/property/set"

// 发布&订阅的JSON数据包内的相关信息
// #define MQTT_POST_MSG_METHOD        thing.service.property.set


#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	
#define CONNECT_MQTT_LED(x)	PCout(13)=(x)?0:1

//变量声明

//函数声明
int Mqtt_Connect_Aliyun(void);											//配置MQTT链接阿里云
int32_t mqtt_connect(char *client_id,char *user_name,char *password);	//MQTT连接服务器
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether);	//MQTT消息订阅
uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos);	//MQTT消息发布
int32_t mqtt_send_heart(void);											//MQTT发送心跳包
void mqtt_report_devices_status(void);									//设备状态上报
void mqtt_report_device_hcho(float hcho_mg_per_cubic_meter);            // 发布甲醛浓度的数值
void mqtt_report_device_pm2_5(float pm2_5_mg_per_cubic_meter);          // 发布PM2.5浓度的数值
void mqtt_report_device_fanspeed(int cur_fan_speed);                    // 发布风扇转速
void mqtt_report_device_lock(bool lock);                                // 发布儿童锁的状态

void mqtt_disconnect(void);												//MQTT无条件断开
void mqtt_send_bytes(uint8_t *buf,uint32_t len);						//MQTT发送数据
void mqtt_msg_handle(void);												//处理MQTT下发数据
uint32_t  mqtt_cjson_parse(char *pbuf);									//解析MQTT下发数据
#endif
