#ifndef __LOG_H__
#define __LOG_H__   

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */


//日志模块总开关
#define __DEBUG_SWITCH                      (1)    
#define __DEBUG_ON_MULTIPLE_THREADS         (0)


#if (__DEBUG_SWITCH) && !defined(_STDIO_H)
    #error You have turned on the LOG system, please to include <stdio.h> !
#endif

#if (__DEBUG_ON_MULTIPLE_THREADS)
    #include <pthread.h>
    #define PTHREAD_SELF()      pthread_self()
#else
    #define PTHREAD_SELF()      (unsigned long)(0)
#endif

//定义日志级别
enum LOG_LEVEL {    
    LOG_LEVEL_OFF = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG
};

// 选择需要打开得日志等级
#define __DEBUG_LEVEL       LOG_LEVEL_DEBUG                

#if (__DEBUG_SWITCH)
    #define DEBUG_LOG(level, fmt, ...)                                      \
        do                                                                  \
        {                                                                   \
            if (level < __DEBUG_LEVEL)                                      \
                break;                                                      \
            printf("[THREAD %ld][FUNC:%s][LINE:%d] => "fmt"\n",             \
                PTHREAD_SELF(), __FUNCTION__, __LINE__, ##__VA_ARGS__);     \
        } while (0);
#else
    #define DEBUG_LOG(level, format, ...)
#endif

#define DEBUG_LOG_INFO(format, ...)             DEBUG_LOG(LOG_LEVEL_INFO,  format, ##__VA_ARGS__)
#define DEBUG_LOG_WARN(format, ...)             DEBUG_LOG(LOG_LEVEL_WARN,  format, ##__VA_ARGS__)
#define DEBUG_LOG_ERROR(format, ...)            DEBUG_LOG(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define DEBUG_LOG_DEBUG(format, ...)            DEBUG_LOG(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif              /** __LOG_H__ **/