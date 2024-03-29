#include "light_sensor.h"
#include "stm32f4xx.h"

// 光敏电阻初始化
void LightSensor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	// 1.打开ADC外设的接口时钟  PF7 -- ADC3_IN5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

	// 2.打开GPIO端口时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	// 3.配置GPIO引脚参数+初始化 注意：引脚模式为模拟模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	// 4.配置ADC外设的参数
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;					 // 独立模式
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;					 // 预分频值  84MHZ /2 = 42MHZ
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;		 // 不使用DMA
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 两次采样的间隔时间
	ADC_CommonInit(&ADC_CommonInitStructure);

	// 5.配置ADC外设的转换精度、数据对齐
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;						// 转换精度  12bit
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;								// 不扫描
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							// 不连续转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 不使用外部触发源
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						// 数据对齐  右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 1;									// 转换序号
	ADC_Init(ADC3, &ADC_InitStructure);

	// 6.配置ADC的规则通道  总转换时间 = 3 + 12 = 15周期
	ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_3Cycles);

	// 7.使能ADC
	ADC_Cmd(ADC3, ENABLE);
}
