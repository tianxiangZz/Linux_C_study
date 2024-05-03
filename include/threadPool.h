#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */



/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define THREAD_POOL_MAX_NUM                     (64)
#define THREAD_POOL_MAX_WAIT_TASK_NUM           (-1)


typedef void (*threadPoolTaskFunc)(void *arg);

struct _threadPool;
typedef struct _threadPool threadPool;



/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

threadPool *threadPoolCreate(int threadnum);
void threadPoolDestroy(void *data);

// void threadPoolStopWaitDone(threadPool *this);
// void threadPoolStop(threadPool *this);

int threadPoolTaskAdd(threadPool *this, threadPoolTaskFunc func, void *arg);




#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __THREAD_POOL_H__ **/