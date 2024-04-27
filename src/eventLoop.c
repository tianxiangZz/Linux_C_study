

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <stdlib.h>
#include <pthread.h>

#include "eventLoop.h"
#include "eventLoop_internal.h"

#include "base.h"

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _eventLoop
{
    void *pollable;
    pollerBase *pollerBase;
    
    // hashmap ->               fd : event
    

    // active event queue
    

    // timer event heap

    int stop;
    const pthread_t pthreadId;
};

/*************************************************************************************************/
/**                                            VARIABLES                                       **/
/*************************************************************************************************/
extern pollerBase epollPollerBase;

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

eventLoop* eventLoopCreate()
{
    eventLoop *this = NULL;

    if (NULL == (this = BASE_CALLOC(1, sizeof(eventLoop))))
    {
        return NULL;
    }

    // 选择 epoll
    this->pollerBase = &epollPollerBase;
    this->pollable = this->pollerBase->create();









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
        if (NULL != this->pollerBase && NULL != this->pollerBase->destroy)
        {
            this->pollerBase->destroy(this->pollerBase);
        }






        THIS_FREE(this);            
    }
    return ;
}



