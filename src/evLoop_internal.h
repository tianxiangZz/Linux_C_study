

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <stdint.h>

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

typedef struct _pollerBase pollerBase;
typedef struct _firedEvents firedEvents;


/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _firedEvents
{
    int fd;
    uint32_t events;
};

struct _pollerBase
{
    void *(*create)();
    void (*destroy)(void *pollable);

    int (*add)(void *pollable, int fd, uint32_t events);
    int (*mod)(void *pollable, int fd, uint32_t events);
    int (*del)(void *pollable, int fd, uint32_t events);

    int (*dispatch)(void *pollable, struct timeval *tv, firedEvents *fired);
};










