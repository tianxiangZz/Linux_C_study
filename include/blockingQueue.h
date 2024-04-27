#ifndef __BLOCKING_QUEUE_H__
#define __BLOCKING_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */



/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

struct _blockingQueue;
typedef struct _blockingQueue blockingQueue;

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

blockingQueue *blockingQueueCreate();
void blockingQueueDestroy(void *data);
int blockingQueuePush(blockingQueue *this, void *data);
void *blockingQueuePop(blockingQueue *this);
int blockingQueueSizeGet(blockingQueue *this);



#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __BLOCKING_QUEUE_H__ **/