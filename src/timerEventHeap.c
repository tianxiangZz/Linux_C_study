

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include "fmacros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <errno.h>

#include "timerEventHeap.h"

#include "base.h"
#include "log.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define TIMER_EVENT_MIN_HEAP_DEFAULT    (16)


#define	TIMER_EVENT_TIME_CMP(tv1, tv2, cmp)	   (	\
	    ((tv1)->tv_sec == (tv2)->tv_sec) ?			\
	    ((tv1)->tv_usec cmp (tv2)->tv_usec) :		\
	    ((tv1)->tv_sec cmp (tv2)->tv_sec)           \
    )

#define TIMER_EVENT_TIME_GREATE(t1, t2)       (TIMER_EVENT_TIME_CMP(t1, t2, >))
	



/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _timerEventHeap
{
    ioEvent ioev;

    timerEvent **minHeap;
    int count;
    int heapSize;
};

/*************************************************************************************************/
/**                                            VARIABLES                                       **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static void timerEventToioEventCallBack(int fd, uint32_t events, void *arg)
{
    return ;
}

static int timerEventHeapSizeUpdate(timerEventHeap *this)
{
    size_t newSize;
    timerEvent **newMinHeap = NULL;

    CHECK_INPARA_ENSURE((NULL != this) && (NULL != this->minHeap));

    newSize = this->heapSize << 1;
    if (NULL == (newMinHeap = BASE_REALLOC(this->minHeap, newSize)))
    {
        return RET_ERROR;
    }

    this->heapSize = newSize;
    this->minHeap = newMinHeap;
    return RET_OK;
}


static int timerEventHeapShiftUp(timerEventHeap *this, size_t index, timerEvent *timerev)
{
    size_t parent = 0;
    CHECK_INPARA_ENSURE((NULL != this) && (NULL != timerev) && (NULL != this->minHeap));

    parent = (index - 1) >> 1;

    while (index && TIMER_EVENT_TIME_GREATE(this->minHeap[parent]->tv, timerev->tv))
    {
        this->minHeap[index] = this->minHeap[parent];
        this->minHeap[index]->index = index;

        index = parent;
        parent = (index - 1) >> 1;
    }
    
    this->minHeap[index] = timerev;
    this->minHeap[index]->index = index;
    return RET_OK;    
}

static int timerEventHeapShiftDown(timerEventHeap *this, size_t index, timerEvent *timerev)
{
    size_t child = 0;
    
    CHECK_INPARA_ENSURE((NULL != this) && (NULL != timerev));

    child = (index + 1) << 1;  // 右孩子

    while (child <= this->count)
    {
        // 不存在右孩子    右孩子比左孩子大   则选左孩子
        if (child == this->count || 
            TIMER_EVENT_TIME_GREATE(this->minHeap[child]->tv, this->minHeap[child - 1]->tv))
        {
            --child;
        }

        if (!(TIMER_EVENT_TIME_GREATE(timerev->tv, this->minHeap[child])))
        {
            break;
        }

        this->minHeap[index] = this->minHeap[child];
        this->minHeap[index]->index = index;


        index = child;
        child = (index + 1) << 1;
    }
    this->minHeap[index] = timerev;
    this->minHeap[index]->index = index;

    return RET_OK;
}


/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

timerEventHeap *timerEventHeapCreate(eventLoop *evLoop)
{
    int timerfd = INVALID_FD;
    timerEventHeap *this = NULL;

    CHECK_INPARA_ENSURE_RETNULL(NULL != evLoop);

    if (NULL == (this = BASE_MALLOC(sizeof(timerEventHeap))))
    {
        return NULL;
    }

    if (INVALID_FD == (timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC)))
    {
        goto error;
    }

    if (RET_OK != ioEventInit(&(this->ioev), evLoop, timerfd, EV_READABLE, timerEventToioEventCallBack, this))
    {
        goto error;
    }

    this->count = 0;
    this->heapSize = TIMER_EVENT_MIN_HEAP_DEFAULT;
    if (NULL == (this->minHeap = BASE_CALLOC(this->heapSize, sizeof(timerEvent *))))
    {
        goto error;
    }

    return this;

error:
    timerEventHeapDestroy(this);
    return NULL;
}

void timerEventHeapDestroy(void *data)
{
    timerEventHeap *this = (timerEventHeap *)data;

    if (NULL != data)
    {
        ioEventDeInit(&(this->ioev));
        BASE_CLOSE_FD(this->ioev.fd);
        BASE_FREE(this->minHeap);
        BASE_FREE(this);
    }
    return ;
}

timerEvent *timerEventHeapTop(timerEventHeap *this)
{
    CHECK_INPARA_ENSURE_RETNULL(NULL != this);

    return this->minHeap[0];
}

int timerEventHeapPush(timerEventHeap *this, timerEvent *timerev)
{
    CHECK_INPARA_ENSURE((NULL != this) && (NULL != timerev));

    if (this->count == this->heapSize)
    {
        if (RET_OK != timerEventHeapSizeUpdate(this))
        {
            return RET_ERROR;
        }
    }

    if (RET_OK != timerEventHeapShiftUp(this, this->count + 1, timerev))
    {
        return RET_ERROR;
    }
    ++ (this->count);
    return RET_OK;
}

int timerEventHeapErase(timerEventHeap *this, timerEvent *timerev)
{
    size_t parent = 0;
    timerEvent *last = NULL;

    CHECK_INPARA_ENSURE((NULL != this) && (NULL != timerev));

	if (-1 != timerev->index)
	{
        last = this->minHeap[this->count - 1];
        parent = (timerev->index - 1) >> 1;

		if (timerev->index > 0 && TIMER_EVENT_TIME_GREATE(this->minHeap[parent]->tv, last->tv))
		{
            timerEventHeapShiftUp(this, timerev->index, last);
        }
		else
		{
            timerEventHeapShiftDown(this, timerev->index, last);
        }
        timerev->index = -1;
        return RET_OK;
	}

    return RET_OK;
}

