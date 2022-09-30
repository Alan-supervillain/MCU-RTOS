#ifndef MYLOG_H__
#define MYLOG_H__
/*我的简单log库*/
#include "bsp_usart.h"
#define LOG_LEVEL_TRACE   5
#define LOG_LEVEL_DEBUG   4
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_ERR     1

#define LOG_LEVEL_MAX     5       //----等级调节

#ifndef LOG_LEVEL
#define LOG_LEVEL  LOG_LEVEL_MAX
#endif

#if LOG_LEVEL >= 1
#define ERR(fmt, arg...)  printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg)    //已经自带了换行符了
#else
#define ERR(fmt, arg...)
#endif

#if LOG_LEVEL >= 2
#define WARN(fmt, arg...) printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg)
#else
#define WARN(fmt, arg...)
#endif

#if LOG_LEVEL >= 3
#define INFO(fmt, arg...) printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg)
#else
#define INFO(fmt, arg...)
#endif

#if LOG_LEVEL >= 4
#define DBG(fmt, arg...) printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg)
#else
#define DBG(fmt, arg...)
#endif

#if LOG_LEVEL >= 5
#define TRACE(fmt, arg...) printf("[%s %s() line%d]:" fmt "\r\n",__FILE__,__func__,__LINE__, ## arg)
#else
#define TRACE(fmt, arg...)
#endif

#endif   /*MYLOG_H__*/

