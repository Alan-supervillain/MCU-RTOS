#include "ADC_drv.h"


/*
**@函数名：ADC_InitConfig
**@函数功能：采集烟雾浓度ADC的初始化使能
**@函数参数：无
**@返回值：无
*/
void ADC_InitConfig()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStruct;
    //1，开启gpioa、adc1时钟，设置pc1为模拟输入
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1|RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_Init(GPIOC,&GPIO_InitStruct);      //

	  //2，复位adc1，设置adc1分频因子
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);     //6分频
	ADC_DeInit(ADC1);
	
	  //3，初始化adc1参数，设置adc1的工作模式以及规则序列的相关信息
	ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;   //独立工作
	ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;   //右对齐
	ADC_InitStruct.ADC_NbrOfChannel=1;  //通道个数
	ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None; 
  ADC_InitStruct.ADC_ScanConvMode=DISABLE;   
	ADC_InitStruct.ADC_ContinuousConvMode=ENABLE;
	
	ADC_Init(ADC1,&ADC_InitStruct);
	
	  //4，校准并使能adc1
		ADC_Cmd(ADC1,ENABLE);     //使能adc1
		ADC_ResetCalibration(ADC1);    //复位校准
		while(SET==ADC_GetResetCalibrationStatus(ADC1));    //等待复位校准完成
		ADC_StartCalibration(ADC1);   //开始校准
		while(SET==ADC_GetCalibrationStatus(ADC1));    //等待校准完成

		
	  //5,配置规则通道参数
		//ADC_RegularChannelConfig(ADC1,ADC_Channel_1,1,ADC_SampleTime_239Cycles5);
		//ADC_RegularChannelConfig(ADC1,ADC_Channel_11,2,ADC_SampleTime_239Cycles5);
		
	  //6，开启软件转换
		//ADC_SoftwareStartConvCmd(ADC1,ENABLE);
		
	  //7，等待转换完成，读取adc值
		//ADC_GetConversionValue(ADC1);
	
}


/*
**@函数名：		Get_Adc
**@函数功能：采集烟雾浓度
**@函数参数：无
**@返回值：采集到的烟雾浓度0~4095
*/
u16 Get_Adc(void)   
{
   //设置指定ADC的规则组通道，一个序列，采样时间
 ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5 ); //ADC1,ADC通道,采样时间为239.5周期          
  
 ADC_SoftwareStartConvCmd(ADC1, ENABLE);  //使能指定的ADC1的软件转换启动功能 
  
 while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
 
 return ADC_GetConversionValue(ADC1); //返回最近一次ADC1规则组的转换结果
}
 
 
u16 Count_AdcVal(u8 ch,u8 times)
{
 u32 temp_val=0;
u8 t;
 for(t=0;t<times;t++)
 {
  temp_val+=Get_Adc();
  delay_ms(5);
 }
 return temp_val/times;;
}


uint16_t GET_RG_Value(void)
{
 uint16_t retdata=0;
 retdata=ADC_GetConversionValue(ADC1);
 return retdata;
}

























/**
* @ 函 数 名：Get_RG_Value
* @ 函数功能：获取光敏传感器的值
* @ 函数参数：无
* @ 返 回 值：获取到的光敏值
*/

//uint16_t Get_dc_Value(void)
//{
//    uint16_t retdata=0;
//	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC))
//    retdata=ADC_GetConversionValue(ADC1);
//    return retdata;

//}



