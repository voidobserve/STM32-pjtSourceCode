#include "interface.h"
#include <stdio.h>
#include <string.h>

#include "oled.h"
#include "key.h"
#include "bmp.h"   // 存放图片数据的文件
#include "dht11.h" // 温湿度传感器
#include "delay.h"
#include "ms1100.h"      // 使用到甲醛浓度的全局变量
#include "dust_sensor.h" // 使用到PM2.5浓度的全局变量
#include "fan.h"         // 使用到风扇转速的全局变量，调节风扇转速的接口

cur_index_t index = CUR_MENU; // 界面索引下标，记录OLED当前显示的界面

// 在菜单选择的功能
// 1--选择了查询温湿度
// 2--选择了查询甲醛浓度
// 3--选择了查询PM2.5浓度
// 4--选择了调节风扇转速
int cursel = 0;
// 在主菜单中，确定按钮是否按下的标志位，0--未按下，1--按下
int enter_flag = 0;

static void Show_Menu(void)
{
    OLED_ShowNum(0, 0, 1, 1, 1);
    OLED_ShowChar(8, 0, '-', 1);
    OLED_ShowChinese(16, 0, "查询温湿度", 1);

    OLED_ShowNum(0, 16, 2, 1, 1);
    OLED_ShowChar(8, 16, '-', 1);
    OLED_ShowChinese(16, 16, "查询甲醛浓度", 1);

    OLED_ShowNum(0, 32, 3, 1, 1);
    OLED_ShowChar(8, 32, '-', 1);
    OLED_ShowChinese(16, 32, "查询", 1);
    OLED_ShowStr(48, 32, "PM", 1);
    OLED_ShowNum(64, 32, 2, 1, 1);
    OLED_ShowChar(72, 32, '.', 1);
    OLED_ShowNum(80, 32, 5, 1, 1);
    OLED_ShowChinese(88, 32, "浓度", 1);

    OLED_ShowNum(0, 48, 4, 1, 1);
    OLED_ShowChar(8, 48, '-', 1);
    OLED_ShowChinese(16, 48, "调节风扇转速", 1);

    // OLED_RefreshGraphic();
}

// 显示当前选择的功能，直接将对应的文字反白显示
// 1--选择了查询温湿度
// 2--选择了查询甲醛浓度
// 3--选择了查询PM2.5浓度
// 4--选择了调节风扇转速
// 其他--没有选择对应的功能，显示默认的目录
// void Show_Cursel_Fun(uint8_t lastsel, uint8_t cursel)
void Show_Cursel_Menu(uint8_t cursel)
{
    Show_Menu(); // 先往显存填入默认的目录的数据

    // 在根据选择，将对应功能的文字反白显示
    switch (cursel)
    {
    case 1:
        OLED_ShowNum(0, 0, 1, 1, 0);
        OLED_ShowChar(8, 0, '-', 0);
        OLED_ShowChinese(16, 0, "查询温湿度", 0);
        break;
    case 2:
        OLED_ShowNum(0, 16, 2, 1, 0);
        OLED_ShowChar(8, 16, '-', 0);
        OLED_ShowChinese(16, 16, "查询甲醛浓度", 0);
        break;
    case 3:
        OLED_ShowNum(0, 32, 3, 1, 0);
        OLED_ShowChar(8, 32, '-', 0);
        OLED_ShowChinese(16, 32, "查询", 0);
        OLED_ShowStr(48, 32, "PM", 0);
        OLED_ShowNum(64, 32, 2, 1, 0);
        OLED_ShowChar(72, 32, '.', 0);
        OLED_ShowNum(80, 32, 5, 1, 0);
        OLED_ShowChinese(88, 32, "浓度", 0);
        break;
    case 4:
        OLED_ShowNum(0, 48, 4, 1, 0);
        OLED_ShowChar(8, 48, '-', 0);
        OLED_ShowChinese(16, 48, "调节风扇转速", 0);
        break;
    default: // 没有选择功能
        break;
    }

    // 修改完成后，刷新显存
    OLED_RefreshGraphic();
}

// OLED显示主界面的处理函数
void Interface_Menu(void)
{
    // 根据键值来判断要反白显示的选择的功能菜单
    if (key)
    {
        switch (key)
        {
        case KEY0_PRES:
            cursel = 0;
            break;
        case KEY1_PRES:
            cursel--;
            // 对cursel进行限制，防止超出空气净化器可选的四个功能
            if (cursel <= 0)
            {
                cursel = 4;
            }
            break;
        case KEY2_PRES:
            cursel++;
            // 对cursel进行限制，防止超出空气净化器可选的四个功能
            if (cursel >= 5)
            {
                cursel = 1;
            }
            break;
        case KEY3_PRES:
            // cursel = 0;
            enter_flag = 1; // 确认按钮按下了
            break;
        default:
            break;
        }
    }

    key = 0; // 清除键值
    Show_Cursel_Menu(cursel);

    if (enter_flag)
    {
        // 根据当前反白显示的功能，修改界面索引
        switch (cursel)
        {
        case 1:
            index = CUR_FUNCTION_SHOW_HUMITURE; // 当前显示的界面是显示温湿度
            // Show_Humiture();
            break;
        case 2:
            index = CUR_FUNCTION_SHOW_HCHO;
            break;
        case 3:
            index = CUR_FUNCTION_SHOW_PM2_5;
            break;
        case 4:
            index = CUR_FUNCTION_SHOW_FAN_SPEED;
            break;
        default:
            break;
        }

        cursel = 0;     // 清除标志位
        enter_flag = 0; // 无论从哪种，都清除该标志位
    }
}

// 让0.96'OLED屏显示DHT11采集到的温湿度
void Show_Humiture(void)
{
    OLED_Clear_Graphic_MEM();

    OLED_ShowBMP(0, 0, 32, 32, temperature_icon);
    OLED_ShowChinese(32, 8, "温度：", 1);

    OLED_ShowBMP(0, 32, 32, 32, humi_icon);
    OLED_ShowChinese(32, 36, "湿度：", 1);

    // =================================
    // 在GB2312编码下，℃这个符号在串口调试助手中去看，会乱码
    // 调试用
    // sprintf((char *)buf, "温度 = %d ℃,湿度=%d %%RH \n\n", dhtbuf[2], dhtbuf[0]);
    // printf("%s\r\n", buf);
    // =================================

    // 直接显示上次采集到的数据，这个变量是全局变量
    OLED_ShowNum(80, 8, dhtbuf[2], 2, 1);
    OLED_ShowChinese(104, 8, "℃", 1);

    OLED_ShowNum(80, 36, dhtbuf[0], 2, 1);
    OLED_ShowStr(104, 36, "%RH", 1);

    OLED_RefreshGraphic();

    if (KEY0_PRES == key)
    {
        // 如果按下了返回按键
        // 修改界面的索引下标，下次将显示主菜单
        index = CUR_MENU;
        OLED_Clear_Graphic_MEM(); // 清空显存
    }
}

// 让0.96'OLED屏显示MS1100采集到的甲醛浓度
void Show_HCHO(void)
{
    uint8_t buf[30] = {0};

    OLED_Clear_Graphic_MEM();

    OLED_ShowChinese(24, 0, "甲醛浓度：", 0);

    sprintf((char *)buf, "%f", hcho_mg_per_cubic_meter);
    strcat((char *)buf, "mg/m^3");

    OLED_ShowStr(8, 24, (char *)buf, 1);

    OLED_RefreshGraphic();

    if (KEY0_PRES == key)
    {
        // 如果按下了返回按键
        // 修改界面的索引下标，下次将显示主菜单
        index = CUR_MENU;
        OLED_Clear_Graphic_MEM(); // 清空显存
    }
}

// 让0.96'OLED屏显示粉尘传感器采集到的PM2.5浓度
void Show_PM2_5(void)
{
    uint8_t buf[30] = {0};

    OLED_Clear_Graphic_MEM();

    OLED_ShowStr(16, 0, "PM", 0);
    OLED_ShowNum(32, 0, 2, 1, 0);
    OLED_ShowChar(40, 0, '.', 0);
    OLED_ShowNum(48, 0, 5, 1, 0);
    OLED_ShowChinese(56, 0, "浓度：", 0);

    sprintf((char *)buf, "%f", pm2_5_mg_per_cubic_meter);
    strcat((char *)buf, "mg/m^3");

    OLED_ShowStr(8, 24, (char *)buf, 1);

    OLED_RefreshGraphic();

    if (KEY0_PRES == key)
    {
        // 如果按下了返回按键
        // 修改界面的索引下标，下次将显示主菜单
        index = CUR_MENU;
        OLED_Clear_Graphic_MEM(); // 清空显存
    }
}

void Show_FAN_Speed(void)
{
    OLED_Clear_Graphic_MEM();

    OLED_ShowBMP(0, 30, 30, 30, fan_bmp);
    OLED_ShowChinese(32, 0, "风扇转速", 0);

    if (KEY1_PRES == key)
    {
        FAN_Speed_dec();
        key = 0;
    }
    else if (KEY2_PRES == key)
    {
        FAN_Speed_Inc();
        key = 0;
    }
    
    switch (cur_fan_speed)
    {
    case CUR_FAN_SPEED_CLOSE:
        OLED_ShowChinese(40, 32, "关闭", 1);
        break;
    case CUR_FAN_SPEED_MIN:
        OLED_ShowChinese(40, 32, "最低速", 1);
        break;

    case CUR_FAN_SPEED_LOW:
        OLED_ShowChinese(40, 32, "低速", 1);
        break;

    case CUR_FAN_SPEED_MID:
        OLED_ShowChinese(40, 32, "适中", 1);
        break;

    case CUR_FAN_SPEED_MAX:
        OLED_ShowChinese(40, 32, "最高速", 1);
        break;

    default:
        break;
    }

    OLED_RefreshGraphic();

    if (KEY0_PRES == key)
    {
        // 如果按下了返回按键
        // 修改界面的索引下标，下次将显示主菜单
        index = CUR_MENU;
        OLED_Clear_Graphic_MEM(); // 清空显存
    }
}

// 根据目录索引下标来切换要显示的界面
void OLED_Show_Interface(void)
{
    if (CUR_MENU == index)
    {
        Interface_Menu();
    }
    else if (CUR_FUNCTION_SHOW_HUMITURE == index)
    {
        Show_Humiture();
    }
    else if (CUR_FUNCTION_SHOW_HCHO == index)
    {
        Show_HCHO();
    }
    else if (CUR_FUNCTION_SHOW_PM2_5 == index)
    {
        Show_PM2_5();
    }
    else
    {
        Show_FAN_Speed();
    }
}
