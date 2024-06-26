

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  

#include "eventLoop.h"
#include "event.h"
#include "base.h"
#include "log.h"


/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/




/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                            VARIABLES                                       **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static void ioEventEnableInLoop(void *ioev)
{
    ioEvent *this = (ioEvent *)ioev;

    if (NULL == this)
    {
        return ;
    }

    if (this->addedInLoop & EVENT_IN_LOOP)
    {
        if (this->events & EV_NOEVENT)
        {
            eventLoopDelioEvent(this->evLoop, this);
            return ;
        }
        eventLoopModioEvent(this->evLoop, this);
        return ;
    }
    else if (!(this->addedInLoop & EVENT_IN_LOOP) && !(this->events & EV_NOEVENT))
    {
        eventLoopAddioEvent(this->evLoop, this);
        return ;
    }
    return ;
}

static void ioEventDisableInLoop(void *ioev)
{
    ioEvent *this = (ioEvent *)ioev;

    if (NULL == this)
    {
        return ;
    }

    if (this->addedInLoop & EVENT_IN_LOOP)
    {
        eventLoopDelioEvent(this->evLoop, this);
        return ;
    }
    return ;
}

static void timerEventEnableInLoop(void *data)
{
    timerEvent *this = (timerEvent *)data;
    if (NULL == data)
    {
        return ;
    }
    eventLoopAddTimerEvent(this->evLoop, this);
    return ;
}

static void timerEventDisableInLoop(void *data)
{
    timerEvent *this = (timerEvent *)data;
    if (NULL == data)
    {
        return ;
    }
    eventLoopDelTimerEvent(this->evLoop, this);
    return ;
}

void iobufferEventCallback(int , uint32_t, void *);

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/












ioEvent *ioEventCreate(struct _eventLoop *evLoop, int fd, uint32_t events, 
    ioEventCallback cb, void *args)
{
    ioEvent *this = NULL
    
    if (NULL == (this = BASE_MALLOC(sizeof(ioEvent))))
    {
        return NULL;
    }

    if (RET_OK != ioEventInit(this, evLoop, fd, events, cb, args))
    {
        BASE_FREE(this);
        return NULL;
    }

    return this;
}

void ioEventDestroy(void *data)
{
    ioEvent *this = (ioEvent *)data;

    if (NULL != data)
    {
        if (EVENT_IN_LOOP & this->addedInLoop)
        {
            ioEventDisable(this);
            while (EVENT_IN_LOOP & this->addedInLoop)
            {
                usleep(1);
            }
        }
        BASE_FREE(this);
    }
    return ;
}

int ioEventEnable(ioEvent *ioev)
{
    CHECK_INPARA_ENSURE(NULL != ioev);
    sendInLoop(ioev->evLoop, ioEventEnableInLoop, ioev);
    return RET_OK;
}

void ioEventDisable(ioEvent *ioev)
{
    CHECK_INPARA_ENSURE(NULL != ioev);
    sendInLoop(ioev->evLoop, ioEventDisableInLoop, ioev);
    return RET_OK;
}

int ioEventInit(ioEvent *ioev, struct _eventLoop *evLoop, int fd, uint32_t events, 
    ioEventCallback cb, void *args)
{
    CHECK_INPARA_ENSURE((NULL != ioev) && (NULL != evLoop) && (INVALID_FD != fd) && (NULL != cb));

    ioev->evLoop = evLoop;
    ioev->fd = fd;
    ioev->events = events;
    ioev->cb = cb;
    ioev->args = args;
    ioev->addedInLoop = !(EVENT_IN_LOOP);

    return RET_OK;
}

void ioEventDeInit(ioEvent *this)
{
    if (NULL != this)
    {
        if (EVENT_IN_LOOP & this->addedInLoop)
        {
            ioEventDisable(this);
            while (EVENT_IN_LOOP & this->addedInLoop)
            {
                usleep(1);
            }
        }
        memset(this, 0x00, sizeof(ioEvent));
    }
}



iobufferEvent *iobufferEventCreate(struct _eventLoop *evLoop, int fd)
{
    iobufferEvent *this = NULL;
    CHECK_INPARA_ENSURE_RETNULL((NULL != evLoop) && (INVALID_FD != fd));

    if (NULL == (this = BASE_CALLOC(1, sizeof(iobufferEvent))))
    {
        return NULL;
    }

    if (RET_OK != ioEventInit(&(this->base), evLoop, fd, EV_NOEVENT, iobufferEventCallback, this))
    {
        goto error;
    }

    if (NULL == (this->input = bufferCreate()) ||
        NULL == (this->ouput = bufferCreate()))
    {
        goto error;
    }

    return this;
error:
    iobufferEventDestroy(this);
    return NULL;
}

int iobufferEventCallbacksSet(iobufferEvent *bufferEvent, ioEventRWCallback readcb, ioEventRWCallback writecb, 
    ioEventErrorCallback errorcb, void *args)
{
    CHECK_INPARA_ENSURE(NULL != bufferEvent);

    bufferEvent->readcb = readcb;
    bufferEvent->writecb = writecb;
    bufferEvent->errorcb = errorcb;
    bufferEvent->args = args;

    return RET_OK;
}

void iobufferEventDestroy(void *data)
{
    iobufferEvent *this = (iobufferEvent *)data;

    if (NULL != data)
    {
        ioEventDeInit(&(this->base));
        BASE_DESTROY(this->input, bufferDestroy);
        BASE_DESTROY(this->output, bufferDestroy);
        BASE_FREE(this);
    }

    return ;
}

int iobufferEventEnableRead(iobufferEvent *this)
{
    CHECK_INPARA_ENSURE(NULL != this);
    this->base.events |= EV_READABLE;
    return ioEventEnable(&(this->base));
}

int iobufferEventEnableWrite(iobufferEvent *this)
{
    CHECK_INPARA_ENSURE(NULL != this);
    this->base.events |= EV_WRITEABLE;
    return ioEventEnable(&(this->base));
}

int iobufferEventDisableRead(iobufferEvent *this)
{
    CHECK_INPARA_ENSURE(NULL != this);
    this->base.events &= ~EV_READABLE;
    return ioEventEnable(&(this->base));
}

int iobufferEventDisableWrite(iobufferEvent *this)
{
    CHECK_INPARA_ENSURE(NULL != this);
    this->base.events &= ~EV_WRITEABLE;
    return ioEventEnable(&(this->base));
}

int iobufferEventDelete(iobufferEvent *this)
{
    CHECK_INPARA_ENSURE(NULL != this);
    return ioEventDisable(&(this->base));    
}




timerEvent *timerEventCreate(struct _eventLoop *evLoop, int flag, 
    struct timeval *tv, timerEventCallback cb, void *args)
{
    struct timeval now = { 0 };
    timerEvent *this = NULL;
    CHECK_INPARA_ENSURE_RETNULL((NULL != evLoop) && (NULL != tv) && (NULL != cb));

    if (NULL == (this = BASE_MALLOC(sizeof(timerEvent))))
    {
        return NULL;
    }

    timeUtilTimeGet(&now);
    timeUtilTimerAdd(tv, &now, &(this->tv));
    this->index = -1;
    this->evLoop = evLoop;
    this->flag = flag;
    this->cb = cb;
    this->args = args;
    return this;
}

void timerEventDestroy(void *data)
{
    timerEvent *this = (timerEvent *)data;
    if (NULL != data)
    {
        if (-1 != this->index)
        {
            timerEventDisable(this);
            while (-1 != this->index)
            {
                usleep(1);
            }
        }
        BASE_FREE(this);
    }
    return ;
}

int timerEventEnable(timerEvent *timerev)
{
    CHECK_INPARA_ENSURE(NULL != timerev);
    sendInLoop(timerev->evLoop, timerEventEnableInLoop, timerev);
    return RET_OK;
}

int timerEventDisable(timerEvent *timerev)
{
    CHECK_INPARA_ENSURE(NULL != timerev);
    sendInLoop(timerev->evLoop, timerEventDisableInLoop, timerev);
    return RET_OK; 
}







