
#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__


#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <sys/time.h>
#include "eventBase.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/


typedef int (*eventLoopForeachEventCb)(const eventLoop *, const eventBase *, void *);
typedef void (*timerEventCb)(timerEventBase *timerEvent);

struct _eventLoop;
typedef struct _eventLoop eventLoop;

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

eventLoop* eventLoopCreate();
void eventLoopDestroy(void *data);


void eventLoopLoop(eventLoop *evLoop);
void eventLoopStop(eventLoop *evLoop);

// io event
int eventLoopAddEvent(eventLoop *evLoop, eventBase *event);
int eventLoopModEvent(eventLoop *evLoop, eventBase *event);             // TODO ...
void eventLoopDelEvent(eventLoop *evLoop, eventBase *event);

// timer event
int eventLoopAddTimerEvent(eventLoop *evLoop, timerEventBase *timerEvent);
void eventLoopDelTimerEvent(eventLoop *evLoop, timerEventBase *timerEvent);




int eventLoopForeachEvents(eventLoop *evLoop, eventLoopForeachEventCb fn, void *arg);




#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __EVENT_LOOP_H__ */