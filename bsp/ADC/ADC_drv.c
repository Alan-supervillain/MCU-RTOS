#include "ADC_drv.h"


/*
**@��������ADC_InitConfig
**@�������ܣ��ɼ�����Ũ��ADC�ĳ�ʼ��ʹ��
**@������������
**@����ֵ����
*/
void ADC_InitConfig()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStruct;
    //1������gpioa��adc1ʱ�ӣ�����pc1Ϊģ������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1|RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_Init(GPIOC,&GPIO_InitStruct);      //

	  //2����λadc1������adc1��Ƶ����
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);     //6��Ƶ
	ADC_DeInit(ADC1);
	
	  //3����ʼ��adc1����������adc1�Ĺ���ģʽ�Լ��������е������Ϣ
	ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;   //��������
	ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;   //�Ҷ���
	ADC_InitStruct.ADC_NbrOfChannel=1;  //ͨ������
	ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None; 
  ADC_InitStruct.ADC_ScanConvMode=DISABLE;   
	ADC_InitStruct.ADC_ContinuousConvMode=ENABLE;
	
	ADC_Init(ADC1,&ADC_InitStruct);
	
	  //4��У׼��ʹ��adc1
		ADC_Cmd(ADC1,ENABLE);     //ʹ��adc1
		ADC_ResetCalibration(ADC1);    //��λУ׼
		while(SET==ADC_GetResetCalibrationStatus(ADC1));    //�ȴ���λУ׼���
		ADC_StartCalibration(ADC1);   //��ʼУ׼
		while(SET==ADC_GetCalibrationStatus(ADC1));    //�ȴ�У׼���

		
	  //5,���ù���ͨ������
		//ADC_RegularChannelConfig(ADC1,ADC_Channel_1,1,ADC_SampleTime_239Cycles5);
		//ADC_RegularChannelConfig(ADC1,ADC_Channel_11,2,ADC_SampleTime_239Cycles5);
		
	  //6���������ת��
		//ADC_SoftwareStartConvCmd(ADC1,ENABLE);
		
	  //7���ȴ�ת����ɣ���ȡadcֵ
		//ADC_GetConversionValue(ADC1);
	
}


/*
**@��������		Get_Adc
**@�������ܣ��ɼ�����Ũ��
**@������������
**@����ֵ���ɼ���������Ũ��0~4095
*/
u16 Get_Adc(void)   
{
   //����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
 ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5 ); //ADC1,ADCͨ��,����ʱ��Ϊ239.5����          
  
 ADC_SoftwareStartConvCmd(ADC1, ENABLE);  //ʹ��ָ����ADC1�����ת���������� 
  
 while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������
 
 return ADC_GetConversionValue(ADC1); //�������һ��ADC1�������ת�����
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
* @ �� �� ����Get_RG_Value
* @ �������ܣ���ȡ������������ֵ
* @ ������������
* @ �� �� ֵ����ȡ���Ĺ���ֵ
*/

//uint16_t Get_dc_Value(void)
//{
//    uint16_t retdata=0;
//	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC))
//    retdata=ADC_GetConversionValue(ADC1);
//    return retdata;

//}



