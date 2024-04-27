
/**
 * 该线程池可扩展
 * 可以设置最大等待任务数量、动态增删线程、等
 **/

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include "threadPool.h"

#include "log.h"
#include "base.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define THREAD_POOL_BLOCKING_QUEUE_INIT(q)  \
    (q)->first = NULL;                      \
    (q)->last = &(q)->first




/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

typedef struct _task
{
    struct _task *next;

    threadPoolTaskFunc func;
    void *arg;
} task;

typedef struct _taskBlockingQueue
{
    task *first;
    task **last;

    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} taskBlockingQueue;

struct _threadPool
{
    pthread_t   *threads;
    int threadnum;
    taskBlockingQueue queue;

    int isRunning;                      /** unused now **/
    int maxWaitTaskNum;                 /** unused now **/
};

/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static void *threadPoolCycleFunc(void *data)
{
    task *curTask = NULL;
    threadPool *this = (threadPool *)data;

    CHECK_INPARA_ENSURE_RETNULL(NULL != this);

    for ( ; ; )
    {
        if (0 != pthread_mutex_lock(&this->queue.mutex))
        {
            return NULL;
        }
        
        while (NULL == (this->queue.first)) 
        {
            if (0 != pthread_cond_wait(&this->queue.cond, &this->queue.mutex))
            {
                pthread_mutex_unlock(&this->queue.mutex);
                return NULL;
            }
        }

        curTask = this->queue.first;
        this->queue.first = curTask->next;

        this->queue.size --;

        if (NULL == this->queue.first && 0 == this->queue.size)
        {
            this->queue.last = &(this->queue.first);
        }

        if (0 != pthread_mutex_unlock(&this->queue.mutex))
        {
            BASE_FREE(curTask);
            return NULL;
        }

        curTask->func(curTask->arg);
        BASE_FREE(curTask);
    }
    return NULL;
}

static void threadPoolExitHandler(void *data)
{
    int *exit = (int *)data;
    *exit = 1;
    pthread_exit(0);
    return ;
}

static int threadPoolTaskPost(threadPool *this, task *argTask)
{
    CHECK_INPARA_ENSURE((NULL != this) && (NULL != argTask));

    if (0 != pthread_mutex_lock(&this->queue.mutex))
    {
        return RET_ERROR;
    }

    if (0 != pthread_cond_signal(&this->queue.cond))
    {
        pthread_mutex_unlock(&this->queue.mutex);
        return RET_ERROR;
    }

    *(this->queue.last) = argTask;
    (this->queue.last) = &(argTask->next);
    (this->queue.size) ++;

    pthread_mutex_unlock(&this->queue.mutex);

    return RET_OK;
}

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

threadPool *threadPoolCreate(int threadnum)
{
    int i = 0;
    threadPool *this = NULL;
    pthread_attr_t  attr = { 0 };

    CHECK_INPARA_ENSURE_RETNULL((0 < threadnum) && (THREAD_POOL_MAX_NUM >= threadnum));

    if (NULL == (this = BASE_CALLOC(1, sizeof(threadPool))))
    {
        DEBUG_LOG_ERROR("threadPoolCreate error: oom!");
        return NULL;
    }

    this->isRunning = false;
    this->maxWaitTaskNum = THREAD_POOL_MAX_WAIT_TASK_NUM;
    this->threadnum = threadnum;

    THREAD_POOL_BLOCKING_QUEUE_INIT(&(this->queue));

    if (0 != pthread_mutex_init(&(this->queue.mutex), NULL) ||
        0 != pthread_cond_init(&(this->queue.cond), NULL))
    {
        DEBUG_LOG_ERROR("queue's mutex/cond init error!");
        goto error;
    }

    if (0 != pthread_attr_init(&attr))
    {
        DEBUG_LOG_ERROR("pthread_attr_init() failed!");
        goto error;
    }

    if (0 != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
    {
        DEBUG_LOG_ERROR("pthread_attr_setdetachstate() failed!");
        goto error;
    }

    /** maybe to => pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN); **/

    if (NULL == (this->threads = BASE_CALLOC(threadnum, sizeof(pthread_t))))
    {
        DEBUG_LOG_ERROR("threads calloc error, maybe oom!");
        goto error;
    }

    for (i = 0; i < threadnum; ++i)
    {
        if (0 != pthread_create(&this->threads[i], &attr, threadPoolCycleFunc, this))
        {
            DEBUG_LOG_ERROR("pthread_create() failed!");
            this->threadnum = i;
            goto error;
        }
    }

    return this;
error:
    pthread_attr_destroy(&attr);
    threadPoolDestroy(this);
    return NULL;
}

void threadPoolDestroy(void *data)
{
    int i = 0;
    volatile int exit;
    task mTask = {NULL, threadPoolExitHandler, (void *)&exit};
    threadPool *this = (threadPool *)data;

    if (NULL == this)
    {
        return ;
    }

    /** 采用这种方式，队列内的task一定会消费完，所以不需要额外销毁task **/
    for (i = 0; i < this->threadnum; ++ i)
    {
        exit = 0;

        if (RET_OK != threadPoolTaskPost(this, &mTask))
        {
            return;
        }

        while (!exit)
        {
            usleep(1);
        }
    }

    pthread_mutex_destroy(&(this->queue.mutex));
    pthread_cond_destroy(&(this->queue.cond));

    BASE_FREE(this->threads);
    BASE_FREE(this);
    return ;
}

// void threadPoolStopWaitDone(threadPool *this);
// void threadPoolStop(threadPool *this);

int threadPoolTaskAdd(threadPool *this, threadPoolTaskFunc func, void *arg)
{
    task *mTask = NULL;

    CHECK_INPARA_ENSURE((NULL != this) && (NULL != func));

    if (NULL == (mTask = BASE_CALLOC(1, sizeof(task))))
    {
        return RET_ERROR;
    }

    mTask->next = NULL;
    mTask->func = func;
    mTask->arg = arg;

    if (RET_OK != threadPoolTaskPost(this, mTask))
    {
        BASE_FREE(mTask);
        return RET_ERROR;
    }

    return RET_OK;
}


/*************************************************************************************************/
/**                                       TEST FUNCTIONS                                      **/
/*************************************************************************************************/

#define THREAD_POOL_TEST
#ifdef THREAD_POOL_TEST

#include <stdatomic.h>


#define TEST_NUM        (100)
#define TEST_PRODUCER   (6)
#define TEST_CONSUMER   (3)

atomic_ulong count = 0; 

void taskFunc(void *data)
{
    count ++;

    if (count == (TEST_NUM * TEST_PRODUCER))
    {
        printf("OK %lu !\n", count);
    }
    return ;
}

void *producerFunc(void *arg)
{
    threadPool *this = (threadPool *)arg;

    if (NULL == arg)
    {
        return NULL;
    }

    for (int i = 0; i < TEST_NUM; ++ i)
    {
        if (RET_OK != threadPoolTaskAdd(this, taskFunc, NULL))
        {
            DEBUG_LOG_ERROR("threadPoolTaskAdd error = %d! ", i);
            return NULL;
        }
    }
    return NULL;
}

void threadPoolTestFunc()
{
    pthread_t tid;
    threadPool *consumer = NULL;
    pthread_attr_t  attr = { 0 };

    atomic_init(&count, 0);

    if (0 != pthread_attr_init(&attr))
    {
        DEBUG_LOG_ERROR("pthread_attr_init() failed!");
        return ;
    }

    if (0 != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
    {
        DEBUG_LOG_ERROR("pthread_attr_setdetachstate() failed!");
        return ;
    }

    if (NULL == (consumer = threadPoolCreate(TEST_CONSUMER)))
    {
        DEBUG_LOG_ERROR("threadPoolCreate() failed!");
        return ;
    }

    for (int i = 0; i < TEST_PRODUCER; ++ i)
    {
        pthread_create(&tid, &attr, producerFunc, consumer);
    }




    sleep(3);


    threadPoolDestroy(consumer);
    return;
}

#endif
