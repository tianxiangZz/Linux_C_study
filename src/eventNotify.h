
#ifndef __EVENT_NOTIFY_H__
#define __EVENT_NOTIFY_H__


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

struct _eventNotify;
typedef struct _eventNotify eventNotify;

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

eventNotify *eventNotifyCreate(eventLoop *evLoop);
void eventNotifyDestroy(void* data);

void eventNotifyEnable(eventNotify *notify);
void eventNotifyCall(eventNotify *notify);

#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __EVENT_NOTIFY_H__ **/