

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include "fmacros.h"

#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <string.h>
#include <errno.h>

#include "timeUtil.h"

#include "eventLoop_internal.h"
#include "base.h"
#include "log.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define EPOLL_INITIAL_NEVENT            (32)
#define EPOLL_MAX_NEVENT                (4096)

#define EPOLL_WAIT_FOREVER              (-1)


#define EPOLL_IS_SUPPORT_CREATE1        (1)                 // 后续补充
#define EPOLL_IS_USING_TIMERFD          (1)                 // 后续补充
#undef EPOLL_IS_SUPPORT_CREATE1
#undef EPOLL_IS_USING_TIMERFD


/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

typedef struct _epoll
{
    // pollerBase base;

    int epfd;
    
    struct epoll_event *events;
    int nevents;

    int timerfd;
} epoll;

/*************************************************************************************************/
/**                                       FUNCTION DECLARATION                                      **/
/*************************************************************************************************/

static void* epollCreate();
static void epolldestroy(void *pollable);

static int epollAddEvent(void *pollable, int fd, uint32_t events);
static int epollModEvent(void *pollable, int fd, uint32_t events);
static int epollDelEvent(void *pollable, int fd, uint32_t events);

static int epollDispatch(void *pollable, struct timeval *tv);


/*************************************************************************************************/
/**                                            VARIABLES                                       **/
/*************************************************************************************************/

pollerBase epollPollerBase = {
    .create = epollCreate,
    .destroy = epolldestroy,

    .add = epollAddEvent,
    .mod = epollModEvent,
    .del = epollDelEvent,

    .dispatch = epollDispatch
};


/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static void* epollCreate()
{
    epoll *this = NULL;

    if (NULL == (this = BASE_CALLOC(1, sizeof(epoll))))
    {
        return NULL;
    }

    this->nevents = EPOLL_INITIAL_NEVENT;
    if (NULL == (this->events = BASE_CALLOC(this->nevents, sizeof(struct epoll_event))))
    {
        goto error;
    }

    // maybe support epoll_create1()     TODO ...  
    if (INVALID_FD == (this->epfd = epoll_create(1024)))
    {
        DEBUG_LOG_DEBUG("epoll_create ERROR!");
        goto error;
    }
    // net close exec


    // maybe not support timerfd()     TODO ...  
    if (INVALID_FD == (this->timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)))
    {
        DEBUG_LOG_DEBUG("timerfd_create ERROR!");
        goto error;
    }

    return this;
error:
    epolldestroy(this);
    return NULL;
}

static void epolldestroy(void *pollable)
{
    epoll *this = (epoll *)pollable;

    if (NULL != this)
    {
        BASE_FREE(this->events);
        BASE_CLOSE_FD(this->epfd);
        BASE_CLOSE_FD(this->timerfd);
        BASE_FREE(this);
    }
    return ;
}

static int epollAddEvent(void *pollable, int fd, uint32_t events)
{
    struct epoll_event ev = {0};
    epoll *this = (epoll *)pollable;

    CHECK_INPARA_ENSURE(((NULL != pollable) && (INVALID_FD != fd) && (EV_NOEVENT != events)));

    ev.events = events;
    ev.data.fd = fd;

    if (RET_ERROR == epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev)) 
    {
        DEBUG_LOG_DEBUG("EPOLL_ADD ERROR!");
        return RET_ERROR;
    }

    return RET_OK;
}

static int epollModEvent(void *pollable, int fd, uint32_t events)
{
    struct epoll_event ev = {0};
    epoll *this = (epoll *)pollable;

    CHECK_INPARA_ENSURE(((NULL != pollable) && (INVALID_FD != fd) && (EV_NOEVENT != events)));

    ev.events = events;
    ev.data.fd = fd;

    if (RET_ERROR == epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev)) 
    {
        DEBUG_LOG_DEBUG("EPOLL_ADD ERROR!");
        return RET_ERROR;
    }

    return RET_OK;
}

static int epollDelEvent(void *pollable, int fd, uint32_t events)
{
    struct epoll_event ev = {0};
    epoll *this = (epoll *)pollable;

    CHECK_INPARA_ENSURE(((NULL != pollable) && (INVALID_FD != fd) && (EV_NOEVENT != events)));

    ev.events = events;
    ev.data.fd = fd;

    if (RET_ERROR == epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev)) 
    {
        DEBUG_LOG_DEBUG("EPOLL_DEL ERROR!");
        return RET_ERROR;
    }

    return RET_OK;
}

static int epollDispatch(void *pollable, struct timeval *tv)
{
    int retVal = 0, timeoutMsec = 0, i = 0;
    epoll *this = (epoll *)pollable;

    CHECK_INPARA_ENSURE((NULL != pollable));

    if (NULL == tv || RET_ERROR == (timeoutMsec = timeUtilTv2Msec(tv)))
    {
        timeoutMsec = EPOLL_WAIT_FOREVER;
    }

    retVal = epoll_wait(this->epfd, this->events, this->nevents, timeoutMsec);
    
    if (retVal == -1 && errno != EINTR)
    {
        DEBUG_LOG_ERROR("epoll_wait error %s!", strerror(errno));
        return RET_ERROR;
    }

    for (i = 0; i < retVal; ++ i)
    {
        // TODO ...
    }

    if (retVal == this->nevents && this->nevents < EPOLL_MAX_NEVENT)
    {
        int nevents = this->nevents * 2;
        struct epoll_event *events = NULL;

        if (NULL != (events = BASE_REALLOC(this->events, nevents * sizeof(struct epoll_event))))
        {
            this->events = events;
            this->nevents = nevents;
        }
    }

    return RET_OK;
}
