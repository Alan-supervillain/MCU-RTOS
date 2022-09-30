#ifndef   DMZIIG_H
#define	  DMZIIG_H

#include "stm32f10x.h"

/*电子秤说明*/                          //---------------------------------------------------------
//校准参数
//因为不同的传感器特性曲线不是很一致，因此，每一个传感器需要矫正这里这个参数才能使测量值很准确。
//当发现测试出来的重量偏大时，增加该数值。
//如果测试出来的重量偏小时，减小改数值。
//该值可以为小数
#define GapValue 400


#define HX711_DOUT_GPIO_PORT    				GPIOC			              /* GPIO端口 */
#define HX711_DOUT_GPIO_CLK 	    			RCC_APB2Periph_GPIOC		/* GPIO端口时钟 */
#define HX711_DOUT_GPIO_PIN							GPIO_Pin_8			       

#define HX711_SCK_GPIO_PORT    					GPIOC			              /* GPIO端口 */
#define HX711_SCK_GPIO_CLK 	    				RCC_APB2Periph_GPIOC		/* GPIO端口时钟 */
#define HX711_SCK_GPIO_PIN							GPIO_Pin_9			        

extern unsigned long Weight_Maopi;
extern long Weight_Shiwu;
extern char Flag_ERROR;

unsigned long HX711_Read(void);
void dmziig_GPIO_Config(void);
void Get_Maopi(void);
void Get_Weight(void);
unsigned long HX711_Read(void);
void dmziig_init(void);


#endif 
