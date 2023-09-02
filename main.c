/*
作者：alan
邮箱：3096141163@qq.com
*/
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "buzzer.h"
#include "bsp_dmziig.h"//电子秤(双拼输入法的拼音)
#include "mylog.h"     //日志库等级版  INFO 一般用于打印些状态信息方便阅读  而WARN是调试用的 警告级别的
#include "bsp_dht11.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "queue.h"
#include "ADC_drv.h"
#include "event_groups.h"
#include "bsp_iwdg.h" 

#define display_Task_EVENT  (0x01 << 0)//设置事件掩码的位0
#define Weight_Task_EVENT   (0x01 << 1)//设置事件掩码的位1
#define MQ2_Task_EVENT      (0x01 << 2)//设置事件掩码的位2

#define waring_numb   900           //烟雾报警设定值
#define waring_temp   40            //高温报警设置值

static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t DHT11_Task_Handle = NULL;      /* Weight_Task任务句柄 */
static TaskHandle_t Weight_Task_Handle = NULL;     /* 显示屏任务句柄 */
static TaskHandle_t display_Task_Handle = NULL;
static TaskHandle_t MQ2_Task_Handle = NULL;        //ADC外设  烟雾传感器任务
/********************************** 内核对象句柄 *********************************/

QueueHandle_t my_Queue =NULL;    //用的FIFO
#define  QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */ //刚好够用
//事件组
static EventGroupHandle_t Event_Handle =NULL;

/******************************* 全局变量声明 ************************************/

DHT11_Data_TypeDef DHT11_Data;
uint16_t somke ;//烟雾浓度

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void DHT11_Task(void* pvParameters);/* TaskDHT11任务实现 */
static void Weight_Task(void* pvParameters);/* Weight_Task_Task任务实现 */
static void display_Task(void* pvParameters);
static void MQ2_Task(void* pvParameters);

static void BSP_Init(void);/* 用于初始化板载相关资源 */

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

  /* 开发板硬件初始化 */
	BSP_Init();
   /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */    
}

/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  taskENTER_CRITICAL();           //进入临界区
  /* 创建 Event_Handle */
  Event_Handle = xEventGroupCreate();
    /* 创建my_Queue */
  my_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
  if(NULL != my_Queue)
    INFO("创建my_Queue消息队列成功!\r\n");
  /*DHT11先获取数据 然后vtaskdelay weight和 display_Task相同优先级*/
  /* 创建DHT11_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )DHT11_Task, /* 任务入口函数 */
                        (const char*    )"DHT11_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )4,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&DHT11_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建DHT11_Task任务成功!\r\n");
  
	/* 创建Weight_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Weight_Task, /* 任务入口函数 */
                        (const char*    )"Weight_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Weight_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建Weight_Task任务成功!\r\n");
  
   xReturn = xTaskCreate((TaskFunction_t )display_Task, /* 任务入口函数 */
                        (const char*    )"display_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )1,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&display_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建display_Task任务成功!\r\n");

   xReturn = xTaskCreate((TaskFunction_t )MQ2_Task, /* 任务入口函数 */
                        (const char*    )"MQ2_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&MQ2_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建MQ2_Task任务成功!\r\n");


  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区
}

//优先级4到1排序了的
static void DHT11_Task(void* parameter)
{	
    EventBits_t r_event;
    while (1)
    {
      if  ( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
			{
				printf("湿度为%d.%d ％RH ，温度为 %d.%d℃ \r\n",\
				DHT11_Data.humi_int,DHT11_Data.humi_deci,DHT11_Data.temp_int,DHT11_Data.temp_deci);
        if(DHT11_Data.temp_int > waring_temp)
        {
            buzzer_on;
            LED_RED;
        }
			}			
		  else
			{
				printf("Read DHT11 ERROR!\r\n");
			}

      //在这个最高优先级的任务里收集其他任务是否有运行到的事情 运行到这会等待事情然后阻塞
      r_event = xEventGroupWaitBits(Event_Handle,  /* 事件对象句柄 */
                                MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT,/* 接收线程感兴趣的事件 */
                                pdTRUE,   /* 函数退出时清除事件位 */
                                pdTRUE,   /* 满足感兴趣的所有事件 */
                                portMAX_DELAY);/* 指定超时时间 */
      //如果有事件位被设置，并且满足调用任务解除阻塞条件，则返回的值是满足调用任务退出阻塞态的事件组的值。如果是由于超时退出，则返回的值将不满足调用任务退出阻塞态的事件组的值。
      if((r_event & (MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT)) == (MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT)) 
      {
        /* 如果接收完成并且正确 */
        printf( "MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT is 运行\n");
        INFO("---喂狗---");
        IWDG_ReloadCounter();
      }
      else{
        printf( "事件错误！\n");
      }
      vTaskDelay(1000);
    }
}

/**********************************************************************
  * @ 函数名  ： Weight_Task
  * @ 功能说明： Weight_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Weight_Task(void* parameter)
{	
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
    uint32_t wei_1 = 0;
    uint32_t wei_2 = 0;
    uint32_t wei_3 = 0;
    uint32_t wei_4 = 0;
    while (1)
    {
      Flag_ERROR =0;
      Get_Weight();			//称重
      Scan_Key();
      //显示当前重量
      if( Flag_ERROR == 1)
      {
        printf("ERROR-->超重\n");
        buzzer_on;                            
        LED_RED;                

      }		
      else
      {       
        wei_4=(Weight_Shiwu/1000);    
        wei_3=(Weight_Shiwu%1000/100);
        wei_2=(Weight_Shiwu%100/10);
        wei_1=(Weight_Shiwu%10);
        printf("%ld%ld%ld%ld g\n",wei_4,wei_3,wei_2,wei_1);
        xReturn = xQueueSend( my_Queue, /* 消息队列的句柄 */
                              &wei_4,/* 发送的消息内容 */
                              0 );        /* 等待时间 0 */
                                                         if(pdPASS == xReturn){INFO("消息wei_4发送成功!\n");}else {INFO("消息wei_4发送失败!\n");}
        xReturn = xQueueSend( my_Queue, &wei_3, 0 );     if(pdPASS == xReturn){INFO("消息wei_3发送成功!\n");}else {INFO("消息wei_3发送失败!\n");}
        xReturn = xQueueSend( my_Queue, &wei_2, 0 );     if(pdPASS == xReturn){INFO("消息wei_2发送成功!\n");}else {INFO("消息wei_2发送失败!\n");}
        xReturn = xQueueSend( my_Queue, &wei_1, 0 );     if(pdPASS == xReturn){INFO("消息wei_1发送成功!\n");}else {INFO("消息wei_1发送失败!\n");}
        //业务逻辑 如果三个传感器都不超标则关闭buzzer 此代码块之前的压力传感器已经不超了
        if((somke<=waring_numb)&&(DHT11_Data.temp_int<waring_temp)){
            buzzer_off;
            LED_RGBOFF;
        }

      INFO("该task 完成一次运行，触发事情！\n" );
			/* 触发一个事件 */
			xEventGroupSetBits(Event_Handle,Weight_Task_EVENT);

        vTaskDelay(500);    //发送太快了 挂起给下一个任务运行机会
			}
    }
}
static void MQ2_Task(void* pvParameters){
    for(;;)
    {
      somke = Get_Adc();
		  printf("烟雾浓度是%d\n",somke);
      if(somke > waring_numb){
          buzzer_on;
          LED_RED;
      }
      
      INFO("该task 完成一次运行，触发事情！\n" );
			/* 触发一个事件 */
			xEventGroupSetBits(Event_Handle,MQ2_Task_EVENT);

      vTaskDelay(500);
    }
}
static void display_Task(void* parameter)
{	
      BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdTRUE */
      uint32_t r_wei;	/* 定义一个接收消息的变量 */
    //启动画面   字的大小弄了16*16
#define line_1  0
      OLED_ShowCHinese(0,line_1,0);//你
      OLED_ShowCHinese(16,line_1,1);//好
      OLED_ShowString(35,line_1,"alan",5);
      OLED_ShowString(50,line_1+2,"/( >,<)/",9);
      OLED_ShowString(0,line_1+3,"https://github.com/Alan-supervillain/MCU-RTOS",46);
      vTaskDelay(1500);
      OLED_Clear();

#define line_2  2
#define line_3  4
#define line_4  6
      //这里是不需要一直刷新的数据   
      OLED_ShowCHinese(0,0,2);//湿
      OLED_ShowCHinese(16,0,3);//度
      OLED_ShowCHinese(32,0,4);//为
      OLED_ShowChar(70,0,46,16);//  . aiisc 是 46

      OLED_ShowCHinese(0,line_2,5);//温
      OLED_ShowCHinese(16,line_2,3);//度
      OLED_ShowCHinese(32,line_2,4);//为*/
      OLED_ShowChar(70,line_2,46,16);//.

      OLED_ShowCHinese(0,line_3,6);//重
      OLED_ShowCHinese(16,line_3,7);//量
      OLED_ShowCHinese(32,line_3,4);//为
      OLED_ShowChar(88,line_3,103,16);  //g的ascii

      OLED_ShowCHinese(0,line_4,8);//作者兰政
      OLED_ShowCHinese(16,line_4,9);
      OLED_ShowChar(32,line_4,58,16);  //：的ascii
      OLED_ShowCHinese(48,line_4,10);
      OLED_ShowCHinese(64,line_4,11);
      u8 x=0;
    while (1)
    {
      OLED_ShowNum(48,0,DHT11_Data.humi_int,2,16);  //湿度整数
      OLED_ShowNum(83,0,DHT11_Data.humi_deci,1,16);  //湿度小数
//这个x是0-127的  而这个y的大小表示下一个行号  牢记而不是demo给的0-63！！！！！
      OLED_ShowNum(48,line_2,DHT11_Data.temp_int,2,16);
      OLED_ShowNum(83,line_2,DHT11_Data.temp_deci,1,16);
      xReturn = xQueueReceive( my_Queue,    /* 消息队列的句柄 */
                                  &r_wei,      /* 发送的消息内容 */
                                  portMAX_DELAY); /* 等待时间 一直等 */                   
      if(pdTRUE == xReturn){
        printf("消息队列接收data is %d\n",r_wei);
      }
      else{
        DBG("数据接收出错,错误代码0x%lx",xReturn);
      }
      OLED_ShowNum(48+x,line_3,r_wei,1,16);
      x+=10;
      if(x>30)x=0;

      INFO("该task 完成一次运行，触发事情!" );
			/* 触发一个事件 */
			xEventGroupSetBits(Event_Handle,display_Task_EVENT);
    }
}
/***********************************************************************
  * @ 函数名  ： BSP_Init
  * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
  * @ 参数    ：   
  * @ 返回值  ： 无
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	LED_GPIO_Config();
	USART_Config();
	Key_GPIO_Config();    
	dmziig_GPIO_Config();
	dmziig_init();	
	buzzer_GPIO_Config();
	DHT11_Init();
	delay_init();	    	 
	OLED_Init();
	ADC_InitConfig();
	OLED_Clear();
  /* 检查是否为独立看门狗复位 */
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  {
    INFO("独立看门狗复位");
    /* 清除标志 */
    RCC_ClearFlag();
  }
  else
  {
    /*不是独立看门狗复位(可能为上电复位或者手动按键复位之类的) */
    INFO("不是独立看门狗复位");
  }
  IWDG_Config(IWDG_Prescaler_64 ,625);// IWDG 1s 超时溢出
	INFO("硬件初始化成功\n");
	INFO("你  好  兰  哥\n");
	INFO("---------------------------------\n");
  WARN("代码可读性规范--下次提交实现\n");
}

/********************************END OF FILE****************************/
