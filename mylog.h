#ifndef MYLOG_H__
#define MYLOG_H__
/*我的简单log库*/
#include "bsp_usart.h"
#define LOG_LEVEL_TRACE   5
#define LOG_LEVEL_INFO    4
#define LOG_LEVEL_DEBUG   3
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_ERR     1

#define LOG_LEVEL_MAX     3      //----等级调节

#ifndef LOG_LEVEL
#define LOG_LEVEL  LOG_LEVEL_MAX
#endif
//已经自带换行符了
//临界区防止printf被打断导致log混乱甚至乱码
//因为这个函数替换不算，宏展开后不算单行了 也就不能省略{}了 编程的时候注意
#define printf(fmt, arg...)  taskENTER_CRITICAL();printf( fmt , ## arg);taskEXIT_CRITICAL()    

#if LOG_LEVEL >= 1
#define ERR(fmt, arg...)  taskENTER_CRITICAL();printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg);taskEXIT_CRITICAL()    
#else
#define ERR(fmt, arg...)
#endif

#if LOG_LEVEL >= 2
#define WARN(fmt, arg...) taskENTER_CRITICAL();printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg);taskEXIT_CRITICAL()
#else
#define WARN(fmt, arg...)
#endif

#if LOG_LEVEL >= 3
#define DBG(fmt, arg...) taskENTER_CRITICAL();printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg);taskEXIT_CRITICAL()
#else
#define DBG(fmt, arg...)
#endif
//用来查看运行状态的信息
#if LOG_LEVEL >= 4
#define INFO(fmt, arg...) taskENTER_CRITICAL();printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg);taskEXIT_CRITICAL()
#else
#define INFO(fmt, arg...)
#endif


#if LOG_LEVEL >= 5
#define TRACE(fmt, arg...) taskENTER_CRITICAL();printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg);taskEXIT_CRITICAL()
#else
#define TRACE(fmt, arg...)
#endif

#endif   /*MYLOG_H__*/

