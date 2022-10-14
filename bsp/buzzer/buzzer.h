#ifndef   BUZZER_H
#define   BUZZER_H

#include "stm32f10x.h"


#define buzzer_GPIO_PORT    							GPIOC			              /* GPIO�˿� */
#define buzzer_GPIO_CLK 	    						RCC_APB2Periph_GPIOC		/* GPIO�˿�ʱ�� */
#define buzzer_GPIO_PIN								GPIO_Pin_0	



#define buzzer_off    \
      GPIO_ResetBits(buzzer_GPIO_PORT, buzzer_GPIO_PIN)

#define buzzer_on    \
      GPIO_SetBits(buzzer_GPIO_PORT, buzzer_GPIO_PIN)

void buzzer_GPIO_Config(void);





#endif /*buzer.h*//*����Ұ��103zet6*/

