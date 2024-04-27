
#ifndef __EVENT_BASE_H__
#define __EVENT_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <stdint.h>

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

struct _eventBase;
typedef struct _eventBase eventBase;                // io event

struct _timerEventBase;
typedef struct _timerEventBase timerEventBase;      // timer event
                                                    // signal event

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

typedef struct _eventCallbackVTable
{
    void (*readCallback)(eventBase *evbase);
    void (*writeCallback)(eventBase *evbase);
    void (*closeCallback)(eventBase *evbase);
    void (*errorCallback)(eventBase *evbase);
} eventCallbackVTable;

struct _eventBase
{
    eventCallbackVTable *evCallback;

    const int fd;
                                    // event pri 
    uint32_t events;                /** Events for listen */     
    uint32_t revent;                /** Triggered events */
};

struct _timerEventBase
{
    timerEventCb cb;

    int flag;
    struct timeval *tv;
};















#ifdef __cplusplus
}
#endif                  /** __cplusplus */


#endif                  /** __EVENT_BASE_H__ */