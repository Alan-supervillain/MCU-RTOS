#include "bsp_dmziig.h" 
#include "buzzer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_usart.h"
#include "core_delay.h" 
//#include  <stdbool.h>
//static unsigned long HX711_Buffer = 0;
unsigned long Weight_Maopi = 0;
long Weight_Shiwu = 0;  //4个字节
//static unsigned char flag = 0;
//static bool Flag_ERROR = 0;//  原来是bit Flag_ERROR = 0;用布尔类型替换51 c中bit
char Flag_ERROR = 0;

void dmziig_init(void)
{
		CPU_TS_Tmr_Delay_MS(1000);//延迟1s等待系统稳定  //没算时间
		Get_Maopi();				//称毛皮重量		
}
void dmziig_GPIO_Config(void)
{		
		
		/*	配置gpio*/
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;
		/*开启相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd( HX711_DOUT_GPIO_CLK | HX711_SCK_GPIO_CLK, ENABLE);
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = HX711_DOUT_GPIO_PIN;	
		/*设置引脚模式为xxxxx*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*调用库函数，初始化GPIO*/
		GPIO_Init(HX711_DOUT_GPIO_PORT, &GPIO_InitStructure);	
	
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = HX711_SCK_GPIO_PIN;
	/*设置引脚模式为xxxx*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		/*调用库函数，初始化GPIO*/
		GPIO_Init(HX711_SCK_GPIO_PORT, &GPIO_InitStructure);
}


//****************************************************
//获取毛皮重量
//****************************************************
void Get_Maopi()
{
	Weight_Maopi = HX711_Read();	
} 


//****************************************************
//称重
//****************************************************
void Get_Weight()
{
	Weight_Shiwu = HX711_Read();
	Weight_Shiwu = Weight_Shiwu - Weight_Maopi;		//获取净重
	if(Weight_Shiwu > 0)			
	{	
		Weight_Shiwu = (unsigned int)((float)Weight_Shiwu/GapValue); 	//计算实物的实际重量
																		
																		
		if(Weight_Shiwu > 5000)		//超重报警
		{
			Flag_ERROR = 1;
		}
		else
		{
			Flag_ERROR = 0;
		}
	}
	else
	{
		Weight_Shiwu = 0;
	//	Flag_ERROR = 1;				//负重报警
	}
	
}


unsigned long HX711_Read(void)	//A增益128
{
	unsigned long count; 
	unsigned char i;  

	GPIO_ResetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
	count=0;
	while(GPIO_ReadInputDataBit(GPIOC, HX711_DOUT_GPIO_PIN))
		; 
	for(i=0;i<24;i++)
	{ 
		GPIO_SetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
		count=count<<1; 
		GPIO_ResetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
		if(GPIO_ReadInputDataBit(GPIOC, HX711_DOUT_GPIO_PIN)==1)
		{
			count++; 
		}
	} 
	GPIO_SetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
	count=count^0x800000;//第25个脉冲下降沿来时，转换数据

	GPIO_ResetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
	return(count);
}

/*********************************************END OF FILE**********************/
