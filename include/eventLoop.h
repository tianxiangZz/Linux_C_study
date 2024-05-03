
#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__


#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <sys/time.h>
#include "event.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

typedef int (*eventLoopForeachEventCb)(const eventLoop *, const eventBase *, void *);

typedef void (*functionInLoop)(void *);

struct _eventLoop;
typedef struct _eventLoop eventLoop;

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

eventLoop* eventLoopCreate();
void eventLoopDestroy(void *data);

void eventLoopispatch(eventLoop *evLoop);
void eventLoopStop(eventLoop *evLoop);




// 内部函数 请勿使用
// io event
int eventLoopAddioEvent(eventLoop *evLoop, ioEvent *ioev);            // TODO ...
int eventLoopModioEvent(eventLoop *evLoop, ioEvent *ioev);             
void eventLoopDelioEvent(eventLoop *evLoop, ioEvent *ioev);

// timer event
int eventLoopAddTimerEvent(eventLoop *evLoop, timerEvent *tev);
void eventLoopDelTimerEvent(eventLoop *evLoop, timerEvent *tev);


void sendInLoop(eventLoop *evLoop, functionInLoop fn, void *arg);


int eventLoopForeachioEvents(eventLoop *evLoop, eventLoopForeachEventCb fn, void *arg);


#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __EVENT_LOOP_H__ */