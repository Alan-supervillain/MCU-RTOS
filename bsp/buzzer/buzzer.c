#include "buzzer.h"
void buzzer_GPIO_Config(void)
{		
		
		/*	配置gpio*/
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;
		/*开启相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd( buzzer_GPIO_CLK , ENABLE);
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = buzzer_GPIO_PIN;
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*调用库函数，初始化GPIO*/
		GPIO_Init(buzzer_GPIO_PORT, &GPIO_InitStructure);	
		
}



