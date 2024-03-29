/*************************************************
 Copyright ? 0010. All rights reserved.

// 文件名
@ File name: ms1100.c -- ms1100甲醛检测模块的驱动程序
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
            ms1100.c -- ms1100甲醛检测模块的驱动程序
        注意：MS1100甲醛检测模块输出的模拟电压值是0~5V，
            要先确定MCU对应的IO口是否有5V容忍

@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#include <math.h>
#include "stm32f4xx.h"
#include "ms1100.h"

// ============================================
// 调试用
#include <stdio.h>
#include "usart.h"
#include "delay.h"
// ============================================

volatile float hcho_mg_per_cubic_meter = 0;

// PA6  ADC12_IN6
// MS1100甲醛传感器初始化
// 将3.3V电压分成了4096份

/**
 * @brief  MS1100甲醛传感器初始化函数
 *
 * @param  无
 * @retval 无
 */
void MS1100_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // 1.打开ADC外设的接口时钟
    MS1100_ADC_RCC_CMD(MS1100_ADC_RCC, ENABLE);

    // 2.打开GPIO端口时钟
    MS1100_RCC_CMD(MS1100_PIN, ENABLE);

    // 3.配置GPIO引脚参数+初始化 注意：引脚模式为模拟模式
    GPIO_InitStructure.GPIO_Pin = MS1100_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(MS1100_PROT, &GPIO_InitStructure);

    // 4.配置ADC外设的参数
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     // 独立模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;                  // 预分频值
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      // 不使用DMA
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 两次采样的间隔时间
    ADC_CommonInit(&ADC_CommonInitStructure);

    // 5.配置ADC外设的转换精度、数据对齐
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      // 转换精度  12bit
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                               // 不扫描
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         // 不连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 不使用外部触发源
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      // 数据对齐  右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;                                  // 转换总次数
    ADC_Init(MS1100_ADC, &ADC_InitStructure);

    // 6.配置ADC的规则通道  总转换时间 = 3 + 12 = 15周期
    ADC_RegularChannelConfig(MS1100_ADC, MS1100_ADC_REGULARCHANNEL, 1, ADC_SampleTime_3Cycles);

    // 7.使能ADC
    ADC_Cmd(MS1100_ADC, ENABLE);
}

/**
 * @brief  将采集到的电压值转换成以ppm为单位表示的浮点数值
 *        函数里面的公式是直接看手册中的图片里0ppm到1ppm的斜率计算的，
 *        因此只能测出0~1ppm，而且误差很大
 *
 * @param  adc_val 从甲醛传感器的模拟输出采集到的电压值
 * @retval ppm 转换好的，以ppm为单位的浮点数值
 */
float MS1100_ADCValToPPM(u16 adc_val)
{
    float ppm = 0;
    ppm = 1.46 * (adc_val * 3.3 / 4096);
    return ppm;
}

float MS1100_PPMToMG(float ppm)
{
    float mg_per_cubic_meter = 0;
    mg_per_cubic_meter = (float)1.341 * ppm;
    return mg_per_cubic_meter;
}

void MS1100_Get_Mg_per_cubic_meter(void)
{
    u16 adc_value = 0;
    float ppm = 0;

    // 开启ADC转换
    ADC_SoftwareStartConv(ADC1);

    // 等待转换完成 EOC标志
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
        ;

    // 读取ADC转换结果  12bit精度 value 0 ~ 4095
    adc_value = ADC_GetConversionValue(ADC1);
    printf("adc_value = %d\r\n", adc_value);
    ppm = MS1100_ADCValToPPM(adc_value);
    printf("ppm: %f\r\n", ppm);

    hcho_mg_per_cubic_meter = MS1100_PPMToMG(ppm);
    printf("甲醛浓度: %f mg/m^3\r\n", hcho_mg_per_cubic_meter);
}

/**
 * @brief  MS1100甲醛模块的测试函数（里面是个死循环）
 *
 * @param  无
 * @retval 无
 */
void MS1100_Test(void)
{
    u16 adc_value = 0;
    float ppm = 0;

    while (1)
    {
        // 开启ADC转换
        ADC_SoftwareStartConv(ADC1);

        // 等待转换完成 EOC标志
        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
            ;

        // 读取ADC转换结果  12bit精度 value 0 ~ 4095
        adc_value = ADC_GetConversionValue(ADC1);
        printf("adc_value = %d\r\n", adc_value);
        ppm = MS1100_ADCValToPPM(adc_value);
        printf("ppm: %f\r\n", ppm);

        delay_ms(1000);
    }
}
