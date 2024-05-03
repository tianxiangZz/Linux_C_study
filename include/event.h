
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
typedef void (*ioEventCallback)(int , uint32_t, void *);

/** io buffer event callback **/
typedef void (*ioEventRWCallback)(ioEvent *, buffer *, void *);
typedef void (*ioEventErrorCallback)(ioEvent *, uint8_t, void *);

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
typedef void (*timerEventCallback)(timerEvent *, void *);

#define TIMER_FLAG_REPEATE          (1 << 0)
#define TIMER_FLAG_ONCE             (1 << 1)

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _ioEvent
{
    ioEventCallback cb;

    int fd;
    uint32_t events;                /** Events for listen */     
    // uint32_t revent;                /** Triggered events */
    
    int addedInLoop;

    struct _eventLoop *evLoop;
    
    void *args;
};

struct _iobufferEvent
{
    ioEvent base;

    ioEventRWCallback readcb;
    ioEventRWCallback writecb;
    ioEventErrorCallback errorcb;

    void *args;
    buffer *input;
    buffer *ouput;
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

ioEvent *ioEventCreate(struct _eventLoop *evLoop, int fd, uint32_t events, 
    ioEventCallback cb, void *args);
void ioEventDestroy(void *data);

int ioEventEnable(ioEvent *ioev);
void ioEventDisable(ioEvent *ioev);

int ioEventInit(ioEvent *ioev, struct _eventLoop *evLoop, int fd, uint32_t events, 
    ioEventCallback cb, void *args);
void ioEventDeInit(ioEvent *ioev);





iobufferEvent *iobufferEventCreate(struct _eventLoop *evLoop, int fd);
int iobufferEventCallbacksSet(iobufferEvent *bufferEvent, ioEventRWCallback readcb, ioEventRWCallback writecb, 
    ioEventErrorCallback errorcb, void *args);
void iobufferEventDestroy(void *data);
int iobufferEventEnableRead(iobufferEvent *this);
int iobufferEventEnableWrite(iobufferEvent *this);
int iobufferEventDisableRead(iobufferEvent *this);
int iobufferEventDisableWrite(iobufferEvent *this);
int iobufferEventDelete(iobufferEvent *this);




timerEvent *timerEventCreate(struct _eventLoop *evLoop, int flag, 
    struct timeval *tv, timerEventCallback cb, void *args);
void timerEventDestroy(void *data);
int timerEventEnable(timerEvent *timerev);
int timerEventDisable(timerEvent *timerev);


#ifdef __cplusplus
}
#endif                  /** __cplusplus */


#endif                  /** __EVENT_BASE_H__ */