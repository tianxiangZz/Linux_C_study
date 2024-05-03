

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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
/**                                            VARIABLES                                       **/
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





iobufferEvent *iobufferEventCreate(struct _eventLoop *evLoop, int fd);
int iobufferEventCallbacksSet(iobufferEvent *bufferEvent, ioEventRWCallback readcb, ioEventRWCallback writecb, 
    ioEventErrorCallback errorcb, void *args);
void iobufferEventDestroy(void *data);
int iobufferEventEnableRead(ioEvent *ioev);
int iobufferEventEnableWrite(ioEvent *ioev);
int iobufferEventDisableRead(ioEvent *ioev);
int iobufferEventDisableWrite(ioEvent *ioev);
int iobufferEventDelete(ioEvent *ioev);




timerEvent *timerEventCreate(struct _eventLoop *evLoop, int flag, 
    struct timeval *tv, timerEventCallback cb, void *args)
{
    timerEvent *this = NULL;
}
void timerEventDestroy(void *data);
int timerEventEnable(timerEvent *timerev);
int timerEventStop(timerEvent *timerev);







