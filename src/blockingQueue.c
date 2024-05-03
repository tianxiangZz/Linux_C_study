/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <pthread.h>

#include "dlist.h"
#include "blockingQueue.h"

#include "log.h"
#include "base.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/



/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _blockingQueue
{
    DList *queue;

    dataDestroyFunc destroyFunc;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

blockingQueue *blockingQueueCreate(dataDestroyFunc func)
{
    blockingQueue *this = NULL;

    if (NULL == (this = BASE_CALLOC(1, sizeof(blockingQueue))))
    {
        return NULL;
    }

    if (NULL == (this->queue = dlist_create(NULL, NULL)))
    {
        goto error;
    }

    if (0 != pthread_mutex_init(this->mutex, NULL) ||
        0 != pthread_cond_init(this->cond, NULL))
    {
        goto error;
    }

    this->destroyFunc = func;
    return this;

error:
    blockingQueueDestroy(this);
    return NULL;
}

void blockingQueueDestroy(void *data)
{
    blockingQueue *this = (blockingQueue *)data;
    if (NULL != this)
    {
        // 销毁链表内的data -> TODO...
        BASE_DESTROY(this->queue, dlist_destroy);
        pthread_mutex_destroy(this->mutex);
        pthread_cond_destroy(this->mutex);
        BASE_FREE(this);
    }
    return ;
}

int blockingQueuePush(blockingQueue *this, void *data)
{
    int ret = 0;

    CHECK_INPARA_ENSURE((NULL != this) && (NULL != this->queue) && (NULL != data));

    pthread_mutex_lock(&this->mutex);

    ret = dlist_append(this->queue, data);

    if (T_RET_OK == ret)
    {
        pthread_cond_signal(&this->cond);
    }

    pthread_mutex_unlock(&this->mutex);
    return (T_RET_OK == ret) ? RET_OK : RET_ERROR;
}

void *blockingQueuePop(blockingQueue *this)
{
    CHECK_INPARA_ENSURE((NULL != this) && (NULL != this->queue));

    pthread_mutex_lock(&this->mutex);

    while (0 == dlist_length(this.queue))
    {
        pthread_cond_wait(&this->cond, &this->mutex);
    }
    
    




}

int blockingQueueSizeGet(blockingQueue *this)
{
    
}


