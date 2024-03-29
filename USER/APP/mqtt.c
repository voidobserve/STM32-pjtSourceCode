#include "mqtt.h"
#include "usart3.h"
#include "dht11.h"
#include "ms1100.h"      // 包含甲醛传感器存放甲醛浓度的全局变量
#include "dust_sensor.h" // 包含粉尘传感器存放PM2.5浓度的全局变量
#include "fan.h"         // 包含风扇转速的全局变量
#include "child_safety_lock.h"       // 包含儿童锁的全局变量

#include "cJSON.h"

char mqtt_post_msg[526];
uint32_t mqtt_tx_len;
const uint8_t g_packet_heart_reply[2] = {0xc0, 0x00};

extern uint8_t dhtbuf[5];

// 配置MQTT链接阿里云
int Mqtt_Connect_Aliyun(void)
{
    int ret = 0;
    // 连接到目标TCP服务器
    ret = esp8266_connect_server("TCP", MQTT_BROKERADDRESS, 1883);
    if (ret)
    {
        printf("esp8266_connect_server fail\r\n");
        return -5;
    }
    printf("esp8266_connect_server success\r\n");
    delay_ms(300);

    // 检测连接状态
    ret = esp8266_check_connection_status();
    if (ret)
    {
        printf("esp8266_check_connection_status fail\r\n");

        // 重新连接热点
        while (Esp8266_Init())
            ;
    }
    printf("esp8266_check_connection_status success\r\n");
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);

    // 进入透传模式
    ret = esp8266_entry_transparent_transmission();
    if (ret)
    {
        printf("esp8266_entry_transparent_transmission fail\r\n");
        return -6;
    }
    printf("esp8266_entry_transparent_transmission success\r\n");
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);

    // MQTT接入云端
    if (mqtt_connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD))
    {
        printf("mqtt_connect fail\r\n");
        return -7;
    }
    printf("mqtt_connect success\r\n");
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);

    // MQTT订阅主题
    if (mqtt_subscribe_topic(MQTT_SUBSCRIBE_TOPIC, 0, 1))
    {
        printf("mqtt_subscribe_topic fail\r\n");
        return -8;
    }

    printf("mqtt_subscribe_topic success\r\n");

    return 0;
}

// MQTT连接服务器的打包函数
int32_t mqtt_connect(char *client_id, char *user_name, char *password)
{
    uint8_t encodedByte = 0;
    uint32_t client_id_len = strlen(client_id);
    uint32_t user_name_len = strlen(user_name);
    uint32_t password_len = strlen(password);
    uint32_t data_len;
    uint32_t cnt = 2;
    uint32_t wait = 0;
    mqtt_tx_len = 0;

    // 可变报头+Payload  每个字段包含两个字节的长度标识
    data_len = 10 + (client_id_len + 2) + (user_name_len + 2) + (password_len + 2);

    // 固定报头
    // 控制报文类型
    Tx3Buffer[mqtt_tx_len++] = 0x10; // MQTT Message Type CONNECT
    // 剩余长度(不包括固定头部)
    do
    {
        encodedByte = data_len % 128;
        data_len = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if (data_len > 0)
            encodedByte = encodedByte | 128;
        Tx3Buffer[mqtt_tx_len++] = encodedByte;
    } while (data_len > 0);

    // 可变报头
    // 协议名
    Tx3Buffer[mqtt_tx_len++] = 0;   // Protocol Name Length MSB
    Tx3Buffer[mqtt_tx_len++] = 4;   // Protocol Name Length LSB
    Tx3Buffer[mqtt_tx_len++] = 'M'; // ASCII Code for M
    Tx3Buffer[mqtt_tx_len++] = 'Q'; // ASCII Code for Q
    Tx3Buffer[mqtt_tx_len++] = 'T'; // ASCII Code for T
    Tx3Buffer[mqtt_tx_len++] = 'T'; // ASCII Code for T
    // 协议级别
    Tx3Buffer[mqtt_tx_len++] = 4; // MQTT Protocol version = 4
    // 连接标志
    Tx3Buffer[mqtt_tx_len++] = 0xc2; // conn flags
    Tx3Buffer[mqtt_tx_len++] = 0;    // Keep-alive Time Length MSB
    Tx3Buffer[mqtt_tx_len++] = 60;   // Keep-alive Time Length LSB  60S心跳包

    Tx3Buffer[mqtt_tx_len++] = BYTE1(client_id_len); // Client ID length MSB
    Tx3Buffer[mqtt_tx_len++] = BYTE0(client_id_len); // Client ID length LSB
    memcpy(&Tx3Buffer[mqtt_tx_len], client_id, client_id_len);
    mqtt_tx_len += client_id_len;

    if (user_name_len > 0)
    {
        Tx3Buffer[mqtt_tx_len++] = BYTE1(user_name_len); // user_name length MSB
        Tx3Buffer[mqtt_tx_len++] = BYTE0(user_name_len); // user_name length LSB
        memcpy(&Tx3Buffer[mqtt_tx_len], user_name, user_name_len);
        mqtt_tx_len += user_name_len;
    }

    if (password_len > 0)
    {
        Tx3Buffer[mqtt_tx_len++] = BYTE1(password_len); // password length MSB
        Tx3Buffer[mqtt_tx_len++] = BYTE0(password_len); // password length LSB
        memcpy(&Tx3Buffer[mqtt_tx_len], password, password_len);
        mqtt_tx_len += password_len;
    }

    while (cnt--)
    {
        memset((void *)Rx3Buffer, 0, sizeof(Rx3Buffer));
        Rx3Counter = 0;

        mqtt_send_bytes(Tx3Buffer, mqtt_tx_len);

        // 等待3s时间
        wait = 3000;

        while (wait--)
        {
            delay_ms(1);

            // 检查连接确认固定报头
            if ((Rx3Buffer[0] == 0x20) && (Rx3Buffer[1] == 0x02))
            {
                if (Rx3Buffer[3] == 0x00)
                {
                    printf("连接已被服务器端接受，连接确认成功\r\n");
                    // 连接成功
                    return 0;
                }
                else
                {
                    switch (Rx3Buffer[3])
                    {
                    case 1:
                        printf("连接已拒绝，不支持的协议版本\r\n");
                        break;
                    case 2:
                        printf("连接已拒绝，不合格的客户端标识符\r\n");
                        break;
                    case 3:
                        printf("连接已拒绝，服务端不可用\r\n");
                        break;
                    case 4:
                        printf("连接已拒绝，无效的用户或密码\r\n");
                        break;
                    case 5:
                        printf("连接已拒绝，未授权\r\n");
                        break;
                    default:
                        printf("未知响应\r\n");
                        break;
                    }
                    return 0;
                }
            }
        }
    }

    return -1;
}

/**
 * @brief  MQTT订阅/取消订阅数据打包函数
 * @param  topic  		主题
 * @param  qos    		消息等级
 * @param  whether: 	订阅/取消订阅请求包
 * @retval 0：成功；
 * 		1：失败；
 */
int32_t mqtt_subscribe_topic(char *topic, uint8_t qos, uint8_t whether)
{
    uint8_t encodedByte = 0;
    uint32_t cnt = 2;
    uint32_t wait = 0;

    uint32_t topiclen = strlen(topic);
    uint32_t data_len = 2 + (topiclen + 2) + (whether ? 1 : 0); // 可变报头的长度（2字节）加上有效载荷的长度

    mqtt_tx_len = 0;

    // 固定报头
    // 控制报文类型
    if (whether)
        Tx3Buffer[mqtt_tx_len++] = 0x82; // 消息类型和标志订阅
    else
        Tx3Buffer[mqtt_tx_len++] = 0xA2; // 取消订阅

    // 剩余长度
    do
    {
        encodedByte = data_len % 128;
        data_len = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if (data_len > 0)
            encodedByte = encodedByte | 128;
        Tx3Buffer[mqtt_tx_len++] = encodedByte;
    } while (data_len > 0);

    // 可变报头
    Tx3Buffer[mqtt_tx_len++] = 0;    // 消息标识符 MSB
    Tx3Buffer[mqtt_tx_len++] = 0x01; // 消息标识符 LSB

    // 有效载荷
    Tx3Buffer[mqtt_tx_len++] = BYTE1(topiclen); // 主题长度 MSB
    Tx3Buffer[mqtt_tx_len++] = BYTE0(topiclen); // 主题长度 LSB
    memcpy(&Tx3Buffer[mqtt_tx_len], topic, topiclen);

    mqtt_tx_len += topiclen;

    if (whether)
    {
        Tx3Buffer[mqtt_tx_len++] = qos; // QoS级别
    }

    while (cnt--)
    {
        Rx3Counter = 0;
        memset((void *)Rx3Buffer, 0, sizeof(Rx3Buffer));
        mqtt_send_bytes(Tx3Buffer, mqtt_tx_len);

        wait = 3000; // 等待3s时间
        while (wait--)
        {
            delay_ms(1);

            // 检查订阅确认报头
            if (Rx3Buffer[0] == 0x90)
            {
                printf("订阅主题确认成功\r\n");

                // 获取剩余长度
                if (Rx3Buffer[1] == 3)
                {
                    printf("Success - Maximum QoS 0 is %02X\r\n", Rx3Buffer[2]);
                    printf("Success - Maximum QoS 2 is %02X\r\n", Rx3Buffer[3]);
                    printf("Failure is %02X\r\n", Rx3Buffer[4]);
                }
                // 获取剩余长度
                if (Rx3Buffer[1] == 2)
                {
                    printf("Success - Maximum QoS 0 is %02X\r\n", Rx3Buffer[2]);
                    printf("Success - Maximum QoS 2 is %02X\r\n", Rx3Buffer[3]);
                }

                // 获取剩余长度
                if (Rx3Buffer[1] == 1)
                {
                    printf("Success - Maximum QoS 0 is %02X\r\n", Rx3Buffer[2]);
                }

                // 订阅成功
                return 0;
            }
        }
    }

    if (cnt)
        return 0; // 订阅成功

    return -1;
}

/**
 * @brief  MQTT订阅/取消订阅数据打包函数
 * @param  topic  		主题
 * @param  message  	消息
 * @param  qos    		消息等级
 * @retval 0：成功；
 * 		1：失败；
 */
uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos)
{
    // static
    uint16_t id = 0;
    uint32_t topicLength = strlen(topic);
    uint32_t messageLength = strlen(message);

    uint32_t data_len;
    uint8_t encodedByte;

    mqtt_tx_len = 0;
    // 有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
    // QOS为0时没有标识符
    // 数据长度             主题名   报文标识符   有效载荷
    if (qos)
        data_len = (2 + topicLength) + 2 + messageLength;
    else
        data_len = (2 + topicLength) + messageLength;

    // 固定报头
    // 控制报文类型
    Tx3Buffer[mqtt_tx_len++] = 0x30; // MQTT Message Type PUBLISH

    // 剩余长度
    do
    {
        encodedByte = data_len % 128;
        data_len = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if (data_len > 0)
            encodedByte = encodedByte | 128;
        Tx3Buffer[mqtt_tx_len++] = encodedByte;
    } while (data_len > 0);

    Tx3Buffer[mqtt_tx_len++] = BYTE1(topicLength); // 主题长度MSB
    Tx3Buffer[mqtt_tx_len++] = BYTE0(topicLength); // 主题长度LSB

    memcpy(&Tx3Buffer[mqtt_tx_len], topic, topicLength); // 拷贝主题

    mqtt_tx_len += topicLength;

    // 报文标识符
    if (qos)
    {
        Tx3Buffer[mqtt_tx_len++] = BYTE1(id);
        Tx3Buffer[mqtt_tx_len++] = BYTE0(id);
        id++;
    }

    memcpy(&Tx3Buffer[mqtt_tx_len], message, messageLength);

    mqtt_tx_len += messageLength;

    mqtt_send_bytes(Tx3Buffer, mqtt_tx_len);

    // Qos等级设置的是00，因此阿里云物联网平台是没有返回响应信息的;
    return mqtt_tx_len;
}

// 设备状态上报
void mqtt_report_devices_status(void)
{
    // 把开发板相关的状态变量利用sprintf函数存放到一个数组里，再把该数组利用MQTT协议打包成消息报文
    // 根据实际平台数据对应的设备信息，更改以下信息；
    sprintf(mqtt_post_msg,
            "{\"method\":\"thing.service.property.set\",\"id\":\"134138643\",\"params\":{\
        \"temperature\":%d,\
        \"humi\":%d\
		},\"version\":\"1.0.0\"}",
            dhtbuf[2], dhtbuf[0]);

    // 上报信息到平台服务器
    mqtt_publish_data(MQTT_PUBLISH_TOPIC, mqtt_post_msg, 0);
    printf("messge publish to aliyun server OK\r\n");
}

void mqtt_report_device_hcho(float hcho_mg_per_cubic_meter)
{
    // 把开发板相关的状态变量利用sprintf函数存放到一个数组里，再把该数组利用MQTT协议打包成消息报文
    // 根据实际平台数据对应的设备信息，更改以下信息；
    sprintf(mqtt_post_msg,
            "{\"method\":\"thing.service.property.set\",\"id\":\"134138643\",\"params\":{\
        \"hcho_mg_per_cubic_meter\":%f,\
		},\"version\":\"1.0.0\"}",
            hcho_mg_per_cubic_meter);

    // 上报信息到平台服务器
    mqtt_publish_data(MQTT_PUBLISH_TOPIC, mqtt_post_msg, 0);
    printf("messge hcho_mg_per_cubic_meter publish to aliyun server OK\r\n");
}

void mqtt_report_device_pm2_5(float pm2_5_mg_per_cubic_meter)
{
    // 把开发板相关的状态变量利用sprintf函数存放到一个数组里，再把该数组利用MQTT协议打包成消息报文
    // 根据实际平台数据对应的设备信息，更改以下信息；
    sprintf(mqtt_post_msg,
            "{\"method\":\"thing.service.property.set\",\"id\":\"134138643\",\"params\":{\
        \"pm2_5_mg_per_cubic_meter\":%f,\
		},\"version\":\"1.0.0\"}",
            pm2_5_mg_per_cubic_meter);

    // 上报信息到平台服务器
    mqtt_publish_data(MQTT_PUBLISH_TOPIC, mqtt_post_msg, 0);
    printf("messge hcho_mg_per_cubic_meter publish to aliyun server OK\r\n");
}

void mqtt_report_device_fanspeed(int cur_fan_speed)
{
    // 把开发板相关的状态变量利用sprintf函数存放到一个数组里，再把该数组利用MQTT协议打包成消息报文
    // 根据实际平台数据对应的设备信息，更改以下信息；
    sprintf(mqtt_post_msg,
            "{\"method\":\"thing.service.property.set\",\"id\":\"134138643\",\"params\":{\
        \"fan_speed\":%d,\
		},\"version\":\"1.0.0\"}",
            cur_fan_speed);

    // 上报信息到平台服务器
    mqtt_publish_data(MQTT_PUBLISH_TOPIC, mqtt_post_msg, 0);
    printf("messge hcho_mg_per_cubic_meter publish to aliyun server OK\r\n");
}

void mqtt_report_device_lock(bool lock)
{
    // 把开发板相关的状态变量利用sprintf函数存放到一个数组里，再把该数组利用MQTT协议打包成消息报文
    // 根据实际平台数据对应的设备信息，更改以下信息；
    sprintf(mqtt_post_msg,
            "{\"method\":\"thing.service.property.set\",\"id\":\"134138643\",\"params\":{\
        \"lock\":%d,\
		},\"version\":\"1.0.0\"}",
            lock);

    // 上报信息到平台服务器
    mqtt_publish_data(MQTT_PUBLISH_TOPIC, mqtt_post_msg, 0);
    printf("messge hcho_mg_per_cubic_meter publish to aliyun server OK\r\n");
}

// MQTT发送数据
void mqtt_send_bytes(uint8_t *buf, uint32_t len)
{
    esp8266_send_bytes(buf, len);
}

// 发送心跳包
int32_t mqtt_send_heart(void)
{
    uint8_t buf[2] = {0xC0, 0x00};
    uint32_t cnt = 2;
    uint32_t wait = 0;

#if 0	
	mqtt_send_bytes(buf,2);
	return 0;
#else
    while (cnt--)
    {
        mqtt_send_bytes(buf, 2);
        memset((void *)Rx3Buffer, 0, sizeof(Rx3Buffer));
        Rx3Counter = 0;

        wait = 3000; // 等待3s时间

        while (wait--)
        {
            delay_ms(1);

            // 检查心跳响应固定报头
            if ((Rx3Buffer[0] == 0xD0) && (Rx3Buffer[1] == 0x00))
            {
                printf("心跳响应确认成功，服务器在线。\r\n");
                return 0;
            }
        }
    }
    printf("心跳响应确认失败，服务器离线\r\n");
    return -1;
#endif
}

// MQTT无条件断开
void mqtt_disconnect(void)
{
    uint8_t buf[2] = {0xe0, 0x00};

    mqtt_send_bytes(buf, 2);

    esp8266_disconnect_server();
}

/*
由于mqtt协议发布消息数据包 = 0x30+剩余长度+01+00+Topic主题名+Json内容，例如通过阿里云物联网平台发送如下
0x30 0xE2 0x01 0x00 /thing/service/property/set{"method":"thing.service.property.set","id":"1597870845","params":{"NO":1,"led1":1,"led2":1},"version":"1.0.0"}
传给cJSON时必须全为字符串，不能有0x00，否则遇到0x00会导致直接结束cJSON的。因此需要自行查找'{'开头的Json内容
*/
// 处理阿里云下发数据
void mqtt_msg_handle(void)
{
    uint8_t i;

    // 等待数据接收完毕
    delay_ms(100);

    // （此处可不作检索）检索接收到阿里云下发的数据是否包含有“"method"”，但是！！！如果字符串中包含\0的话，strstr不会查找到最后就会返回null；。
    // if( ! esp8266_find_str_in_rx_packet("method",5000));
    {
        for (i = 0; i < Rx3Counter; i++)
        {
            // 检索'{'
            if (Rx3Buffer[i] == '{')
            {
                // 解析成功，则退出
                if (!mqtt_cjson_parse((char *)&Rx3Buffer[i]))
                    break;
            }
        }
        // 串口接收数据位置置0,即清空数组
        Rx3Counter = 0;
        Rx3End = 0;
    }
}

// 解析MQTT下发数据
/*{
    "method":"thing.service.property.set",
    "id":"1597870845",
    "params":{
        "NO":1,
        "led1":1,
        "led2":1
    },
    "version":"1.0.0"
}
*/

#if 1
uint32_t mqtt_cjson_parse(char *pbuf)
{
    cJSON *Json = NULL, *Method = NULL, *Id = NULL, *Params = NULL, *Item = NULL;

    char *psrt = pbuf;

    // 解析数据包
    Json = cJSON_Parse(psrt);
    if (Json == NULL) // 检测Json数据包是否存在语法上的错误，返回NULL表示数据包无效
    {
        cJSON_Delete(Json);

        // 打印数据包语法错误的位置
        printf("Error before: [%s] \r\n", cJSON_GetErrorPtr());
        return 1;
    }
    else
    {
        // 匹配子对象 method
        if ((Method = cJSON_GetObjectItem(Json, "method")) != NULL)
        {
            printf("---------------------------------method----------------------------\r\n");
            printf("%s: %s \r\n", Method->string, Method->valuestring);
        }
        // 匹配子对象 id
        if ((Id = cJSON_GetObjectItem(Json, "id")) != NULL)
        {
            printf("-----------------------------------id------------------------------\r\n");
            printf("%s: %s \r\n", Id->string, Id->valuestring);
        }

        // 匹配子对象 params
        if ((Params = cJSON_GetObjectItem(Json, "params")) != NULL)
        {
            printf("---------------------------------params----------------------------\r\n");

            if ((Item = cJSON_GetObjectItem(Params, "fan_speed")) != NULL) // 匹配子对象2中的成员 "fan_speed"
            {
                printf("%s: %d \r\n", Item->string, Item->valueint);

                // 控制风扇转速
                cur_fan_speed = Item->valueint;
            }
            if ((Item = cJSON_GetObjectItem(Params, "lock")) != NULL) // 匹配子对象2中的成员 "lock"
            {
                printf("%s: %d \r\n", Item->string, Item->valueint);

                // 控制风扇转速
                lock = Item->valueint;
            }
        }
    }

    // 释放cJSON_Parse()分配出来的内存空间
    cJSON_Delete(Json);
    Json = NULL;

    return 0;
}
#endif
