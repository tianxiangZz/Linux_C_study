

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <errno.h>

#include "eventNotify.h"
#include "base.h"
#include "log.h"

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _eventNotify
{
    ioEvent ioev;
};

/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static void eventfdToioEventCallback(int , uint32_t, void *);

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

eventNotify *eventNotifyCreate(eventLoop *evLoop)
{
    int evfd = INVALID_FD;
    eventNotify *this = NULL;

    CHECK_INPARA_ENSURE_RETNULL(NULL != evLoop);

    if (NULL == (this = BASE_MALLOC(sizeof(eventNotify))))
    {
        return NULL;
    }

    if (INVALID_FD == (evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)))
    {
        goto error;
    }

    if (RET_OK != ioEventInit(&(this->ioev), evLoop, evfd, EV_READABLE, eventfdToioEventCallback, this))
    {
        goto error;
    }

    return this;
error:
    eventNotifyDestroy(this);
    return NULL;
}

void eventNotifyDestroy(void* data)
{
    eventNotify *this = (eventNotify *)data;
    if (NULL != data)
    {
        ioEventDeInit(&(this->ioev));
        BASE_CLOSE_FD(this->ioev.fd);
        BASE_FREE(this);
    }
    return ;
}

void eventNotifyEnable(eventNotify *this)
{
    if (NULL != notify)
    {
        ioEventEnable(&(this->ioev));
    }
    return ;
}

void eventNotifyCall(eventNotify *this)
{
    eventfd_t val = 1;
    if (NULL != this)
    {
        if (sizeof(eventfd_t) != (this->ioev.fd, val))
        {
            DEBUG_LOG_ERROR("error!");
        }
    }
    return ;
}

