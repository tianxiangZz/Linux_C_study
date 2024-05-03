

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <stdlib.h>
#include <pthread.h>

#include "timerEventHeap.h"
#include "eventNotify.h"

#include "eventLoop.h"
#include "eventLoop_internal.h"

#include "base.h"
#include "log.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define EVLOOP_DEFAULT_IOEVENTS_NUM         (32)
#define EVLOOP_DEFAULT_NOTIFY_IOEVENT_NUM         (1)
#define EVLOOP_DEFAULT_TIMER_IOEVENT_NUM          (1)
#define EVLOOP_DEFAULT_SIGNAL_IOEVENT_NUM         (0)           // 1 或者 2 

#define EVLOOP_INTERNAL_IOEVENTS_NUM                (3)
#define EVLOOP_DEFAULT_FIREDEVENTS_NUM      ((EVLOOP_DEFAULT_IOEVENTS_NUM) + (EVLOOP_INTERNAL_IOEVENTS_NUM))

#define EVENT_LOOP_PENDING_FUNCTIONS_QUEUE_INIT(q)      \
    (q)->first = NULL;                                  \
    (q)->last = &(q)->first


#define EVLOOP_PTHREAD_ID_GET()             (pthread_self())


/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

typedef struct _pendingFunctin
{
    struct _pendingFunctin *next;

    functionInLoop func;
    void *args;
} pendingFunctin;

typedef struct _pendingFunctinsQueue
{
    pendingFunctin *first;
    pendingFunctin **last;

    int count;
    pthread_mutex_t mutex;
} pendingFunctinsQueue;

struct _eventLoop
{
    void *pollable;
    pollerBase *pollerBase;
    
    firedEvent *fireds;
    int firedSize;

    ioEvent **ioEvents;
    int ioEventSize;
    int ioEventCount;

    timerEventHeap *timerEvents;
    
    eventNotify *notify;

    int isCallPendingFunction;
    pendingFunctinsQueue pendingFuncs;

    int running;               
    const pthread_t pthreadId;
};

/*************************************************************************************************/
/**                                            VARIABLES                                       **/
/*************************************************************************************************/
extern pollerBase epollPollerBase;

/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static int eventLoopIoEventSizeUpdate(eventLoop *evLoop)
{
    size_t newSize = 0;
    ioEvent **newIoEvents = NULL;
    firedEvent *newfireds = NULL;

    CHECK_INPARA_ENSURE(NULL != eventLoop);

    newSize = evLoop->ioEventSize << 1;

    if (NULL == (newIoEvents = BASE_REALLOC(evLoop->ioEvents, newSize * (sizeof(ioEvent *)))))
    {
        return RET_ERROR;
    }

    evLoop->ioEvents = newIoEvents;
    evLoop->ioEventSize = newSize;

    newSize = newSize + EVLOOP_INTERNAL_IOEVENTS_NUM;
    if (NULL == (newfireds = BASE_REALLOC(evLoop->fireds, newSize * (sizeof(firedEvent)))))
    {
        return RET_ERROR;
    }

    evLoop->fireds = newfireds;
    evLoop->firedSize = newSize;
    return RET_OK;
}

static int eventLoopPendingFunctionPush(pendingFunctinsQueue *queue, pendingFunctin *func)
{
    CHECK_INPARA_ENSURE((NULL != queue) && (NULL != func));

    if (0 != pthread_mutex_lock(&(queue->mutex)))
    {
        return RET_ERROR;
    }

    *(queue->last) = func;
    (queue->last) = &(func->next);
    ++ (queue->count); 

    pthread_mutex_unlock(&(queue->mutex));
    return RET_OK;
}

static void eventLoopPendingFunctionsDoTask(pendingFunctinsQueue *queue)
{
    size_t size = 0;
    pendingFunctin *first = NULL;
    pendingFunctin **last = NULL;
    pendingFunctin *curFunc = NULL;

    if (NULL == queue)
    {
        return ;
    }

    if (0 != pthread_mutex_lock(&(queue->mutex)))
    {
        return ;
    }

    first = queue->first;
    last = queue->last;
    size = queue->count;

    queue->first = NULL;
    queue->last = &(queue->first);

    pthread_mutex_unlock(&(queue->mutex));

    // 先取来在慢慢执行
    for ( ; size > 0 ; -- size)
    {
        curFunc = first;
        first = curFunc->next;

        curFunc->func(curFunc->args);

        BASE_FREE(curFunc);
    }

    return ;    
}

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

eventLoop* eventLoopCreate()
{
    eventLoop *this = NULL;

    if (NULL == (this = BASE_MALLOC(sizeof(eventLoop))))
    {
        return NULL;
    }

    // 选择 epoll
    this->pollerBase = &epollPollerBase;
    if (NULL == (this->pollable = this->pollerBase->create()))
    {
        goto error;
    }

    this->running = 0;
    this->pthreadId = EVLOOP_PTHREAD_ID_GET();

    this->ioEventCount = 0;
    this->ioEventSize = EVLOOP_DEFAULT_IOEVENTS_NUM;
    if (NULL == (this->ioEvents = BASE_CALLOC(this->ioEventSize, sizeof(ioEvent *))))
    {
        goto error;
    }

    this->firedSize = EVLOOP_DEFAULT_FIREDEVENTS_NUM;
    if (NULL == (this->fireds = BASE_MALLOC((this->firedSize) * sizeof(firedEvent))))
    {
        goto error;
    }

    this->isCallPendingFunction = 0;
    EVENT_LOOP_PENDING_FUNCTIONS_QUEUE_INIT(&(this->pendingFuncs));
    this->pendingFuncs.count = 0;
    if (0 != pthread_mutex_init(&(this->pendingFuncs.mutex), NULL))
    {
        goto error;
    }

    // timer io
    if (NULL == (this->timerEvents = timerEventHeapCreate(this)))
    {
        goto error;
    }
    // notify io
    if (NULL == (this->notify = eventNotifyCreate(this)))
    {
        goto error;
    }

    return this;
error:
    eventLoopDestroy(this);
    return NULL;
}

void eventLoopDestroy(void *data)
{
    eventLoop *this = (eventLoop *)data;

    if (NULL != this)
    {
        if (this->running)
        {

        }
        // foreach pendingfunc
        pthread_mutex_destroy(&(this->pendingFuncs.mutex));
        BASE_DESTROY(this->notify, eventNotifyDestroy);
        BASE_DESTROY(this->timerEvents, timerEventHeapDestroy);
        BASE_FREE(this->fireds);
        BASE_FREE(this->ioEvents);
        BASE_DESTROY(this->pollerBase, this->pollerBase->destroy);
        BASE_FREE(this);            
    }
    return ;
}

void eventLoopispatch(eventLoop *evLoop)
{
    int retEvents = 0, i = 0;
    firedEvent *firedEvent = NULL;
    ioEvent *ioev = NULL;

    if (NULL == evLoop)
    {
        return ;
    }
    
    evLoop->running = 1;
    while (1)
    {
        retEvents = evLoop->pollerBase->dispatch(evLoop->pollable, NULL, evLoop->fireds);
        if (-1 == retEvents)
        {
            break;
        }
        for (i = 0; i < retEvents; ++i)
        {
            firedEvent = evLoop->fireds[i];
            ioev = evLoop->ioEvents[firedEvent->fd];
            ioev->cb(firedEvent->fd, firedEvent->events, ioev->args);
        }

        evLoop->isCallPendingFunction = 1;
        eventLoopPendingFunctionsDoTask(&(evLoop->pendingFuncs));
        evLoop->isCallPendingFunction = 0;
    }
    evLoop->running = 0;
    return ;
}

void eventLoopStop(eventLoop *evLoop);

// io event
int eventLoopAddioEvent(eventLoop *evLoop, ioEvent *ioev)
{
    CHECK_INPARA_ENSURE((NULL != evLoop) && (NULL != ioev));

    if (evLoop->ioEventCount == evLoop->ioEventSize)
    {
        if (RET_OK != eventLoopIoEventSizeUpdate(evLoop))
        {
            return RET_ERROR;
        }
    }

    if (RET_OK != evLoop->pollerBase->add(evLoop->pollable, ioev->fd, ioev->events))
    {
        return RET_ERROR;
    }

    evLoop->ioEvents[ioev->fd] = ioev;
    return RET_OK;
}

int eventLoopModioEvent(eventLoop *evLoop, ioEvent *ioev)
{
    CHECK_INPARA_ENSURE((NULL != evLoop) && (NULL != ioev));

    if (evLoop->ioEvents[ioev->fd] != ioev)
    {
        return RET_ERROR;
    }

    if (RET_OK != evLoop->pollerBase->mod(evLoop->pollable, ioev->fd, ioev->events))
    {
        return RET_ERROR;
    }

    return RET_OK;
}

void eventLoopDelioEvent(eventLoop *evLoop, ioEvent *ioev)
{
    CHECK_INPARA_ENSURE((NULL != evLoop) && (NULL != ioev));

    if (evLoop->ioEvents[ioev->fd] != ioev)
    {
        return RET_ERROR;
    }

    if (RET_OK != evLoop->pollerBase->del(evLoop->pollable, ioev->fd, ioev->events))
    {
        return RET_ERROR;
    }

    evLoop->ioEvents[ioev->fd] = NULL;

    return RET_OK;
}

// timer event
int eventLoopAddTimerEvent(eventLoop *evLoop, timerEvent *timerev)
{
    CHECK_INPARA_ENSURE((NULL != evLoop) && (NULL != timerev));

    if (RET_OK != timerEventHeapPush(evLoop->timerEvents, timerev))
    {
        return RET_ERROR;
    }
    return RET_OK;
}

void eventLoopDelTimerEvent(eventLoop *evLoop, timerEvent *timerev)
{
    CHECK_INPARA_ENSURE((NULL != evLoop) && (NULL != timerev));

    if (RET_OK != timerEventHeapErase(evLoop->timerEvents, timerev))
    {
        return RET_ERROR;
    }
    return RET_OK;
}


void sendInLoop(eventLoop *evLoop, functionInLoop fn, void *args)
{
    pendingFunctin *func = NULL;
    if (NULL == evLoop || NULL == fn)
    {
        return ;
    }

    if (EVLOOP_PTHREAD_ID_GET() == evLoop->pthreadId)
    {
        fn(args);
    }
    else
    {
        if (NULL == (func = BASE_MALLOC(sizeof(pendingFunctin))))
        {
            return ;
        }
        func->next = NULL;
        func->func = fn;
        func->args = args;

        if (RET_OK != eventLoopPendingFunctionPush(&(evLoop->pendingFuncs), func))
        {
            BASE_FREE(func);
            return ;
        }
    }
    return ;
}


int eventLoopForeachioEvents(eventLoop *evLoop, eventLoopForeachEventCb fn, void *arg);


