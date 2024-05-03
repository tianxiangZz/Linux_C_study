
#ifndef __TIMER_EVENT_HEAP_H__
#define __TIMER_EVENT_HEAP_H__


#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include "eventLoop.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

struct _timerEventHeap;
typedef struct _timerEventHeap timerEventHeap;              // 本质是一个timerfd的IO事件

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

timerEventHeap *timerEventHeapCreate(eventLoop *evLoop);
void timerEventHeapDestroy(void *data);

timerEvent *timerEventHeapTop(timerEventHeap *this);

int timerEventHeapPush(timerEventHeap *this, timerEvent *evtime);
int timerEventHeapErase(timerEventHeap *this, timerEvent *evtime);

#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __TIMER_EVENT_HEAP_H__ **/