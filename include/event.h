
#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <sys/time.h>
#include <stdint.h>
#include "buffer.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

struct _eventLoop;

struct _ioEvent;
typedef struct _ioEvent ioEvent;                // ioEvent

struct _iobufferEvent;
typedef struct _iobufferEvent iobufferEvent;    // iobufferEvent extend from ioEvent

struct _timerEvent;
typedef struct _timerEvent timerEvent;          // timer event

struct _signalEvent;
typedef struct _signalEvent signalEvent;        // signal event TODO...

/** io event callback **/
typedef void (*ioEventCallback)(int , void *);

/** io buffer event callback **/
typedef void (*iobufferEventRWCallback)(buffer *, void *);
typedef void (*iobufferEventErrorCallback)(uint8_t, void *);

#define EVENT_IN_LOOP      (1)
/** io event **/
#define EV_NOEVENT              (0x00)
#define EV_READABLE             (1 << 0)
#define EV_WRITEABLE            (1 << 1)
#define EV_ET                   (1 << 2)
#define EV_ERROR                (1 << 3)
#define EV_CLOSE                (1 << 4)

/** ioevent error type **/
#define IO_EVENT_READ_ERROR	    (1 << 0)
#define IO_EVENT_WRITE_ERROR	(1 << 1)
#define IO_EVENT_EOF		    (1 << 2)
#define IO_EVENT_CLOSE		    (1 << 3)
#define IO_EVENT_ERROR		    (1 << 4)

/** timer event callback **/
typedef void (*timerEventCallback)(void *);

#define TIMER_FLAG_REPEATE          (1 << 0)
#define TIMER_FLAG_ONCE             (1 << 1)

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _ioEvent
{
    int fd;
    uint32_t events;                /** Events for listen */     
    // uint32_t revent;                /** Triggered events */
    
    int addedInLoop;
    struct _eventLoop *evLoop;
    
    ioEventCallback readcb;
    ioEventCallback writecb;
    ioEventCallback closecb;
    ioEventCallback errorcb;
    void *args;
};

struct _iobufferEvent
{
    ioEvent base;

    buffer *input;
    buffer *ouput;

    iobufferEventRWCallback readcb;
    iobufferEventRWCallback writecb;
    iobufferEventErrorCallback errorcb;
    void *args;
};

struct _timerEvent
{
    int flag;
    struct timeval tv;

    struct _eventLoop *evLoop;
    int index;                      // 在heap中的位置
    timerEventCallback cb;
    void *args;
};


/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

ioEvent *ioEventCreate(struct _eventLoop *evLoop, int fd);
void ioEventDestroy(void *data);
int ioEventCallbacksSet(ioEvent *ioev, ioEventCallback readcb, ioEventCallback writecb, ioEventCallback errorcb, void *args);
void ioEventReadEnable(ioEvent *ioev);
void ioEventReadDisable(ioEvent *ioev);
void ioEventWriteEnable(ioEvent *ioev);
void ioEventWriteDisable(ioEvent *ioev);
void ioEventAllDisable(ioEvent *ioev);

int ioEventInit(ioEvent *ioev, struct _eventLoop *evLoop, int fd);
void ioEventDeInit(ioEvent *ioev);

iobufferEvent *iobufferEventCreate(struct _eventLoop *evLoop, int fd);
void iobufferEventDestroy(void *data);
int iobufferEventCallbacksSet(iobufferEvent *bufferEvent, iobufferEventRWCallback readcb, iobufferEventRWCallback writecb, 
    iobufferEventErrorCallback errorcb, void *args);
int iobufferEventReadEnable(iobufferEvent *this);
int iobufferEventWriteEnable(iobufferEvent *this);
int iobufferEventReadDisable(iobufferEvent *this);
int iobufferEventWriteDisable(iobufferEvent *this);
int iobufferEventAllDisable(iobufferEvent *this);

timerEvent *timerEventCreate(struct _eventLoop *evLoop, int flag, 
    struct timeval *tv, timerEventCallback cb, void *args);
void timerEventDestroy(void *data);
int timerEventEnable(timerEvent *timerev);
int timerEventDisable(timerEvent *timerev);


#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __EVENT_H__ */