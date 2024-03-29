#include "dust_sensor.h"
#include "stm32f4xx.h"
#include "delay.h" // 延时函数接口

#include <stdio.h>
#include "usart.h"

volatile float pm2_5_mg_per_cubic_meter = 0;

// 粉尘传感器初始化
void DustSensor_Init(void)
{
    // 控制粉尘的红外LED的 ILED初始化
    // |--使用PG12

    GPIO_InitTypeDef GPIO_InitStructure;

    // 时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 推完输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   // 上拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    // 设置为低电平--不开启粉尘传感器的LED
    GPIO_ResetBits(GPIOG, GPIO_Pin_12);

    // 负责检测粉尘传感器输出电压值的端口初始化（初始化GPIO和ADC）
    // |--目前使用PA4--ADC2_IN4
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // 1.打开ADC外设的接口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

    // 2.打开GPIO端口时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 3.配置GPIO引脚参数+初始化 注意：引脚模式为模拟模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4.配置ADC外设的参数
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     // 独立模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;                  // 预分频值  84MHZ /2 = 42MHZ
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
    ADC_Init(ADC2, &ADC_InitStructure);

    // 6.配置ADC的规则通道  总转换时间 = 3 + 12 = 15周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 1, ADC_SampleTime_3Cycles);

    // 7.使能ADC
    ADC_Cmd(ADC2, ENABLE);
}

void DustSensor_GetVal(void)
{
    u16 adc_value = 0;

    DUST_LED_SET(0);
    delay_us(5);

    DUST_LED_SET(1);
    delay_us(280);

    // 开启ADC转换
    ADC_SoftwareStartConv(ADC2);

    // 等待转换完成 EOC标志
    while (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET)
        ;

    // 读取ADC转换结果  12bit精度 value 0 ~ 4095
    adc_value = ADC_GetConversionValue(ADC2);

    DUST_LED_SET(0);
    delay_us(5);

    printf("adc_value = %d\r\n", adc_value);

    pm2_5_mg_per_cubic_meter = adc_value * 3.3 * 5.8 / 4096;
    printf("粉尘浓度： %lf mg/m^3\r\n", pm2_5_mg_per_cubic_meter);
}

void DustSensor_Test(void)
{
    u16 adc_value = 0;

    while (1)
    {
        DUST_LED_SET(0);
        delay_us(5);

        DUST_LED_SET(1);
        delay_us(280);

        // 开启ADC转换
        ADC_SoftwareStartConv(ADC2);

        // 等待转换完成 EOC标志
        while (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET)
            ;

        // 读取ADC转换结果  12bit精度 value 0 ~ 4095
        adc_value = ADC_GetConversionValue(ADC2);

        DUST_LED_SET(0);
        delay_us(5);

        printf("adc_value = %d\r\n", adc_value);

        printf("粉尘浓度： %lf mg/m^3\r\n", adc_value * 3.3 * 5.8 / 4096);
        delay_ms(1000);
    }
}
