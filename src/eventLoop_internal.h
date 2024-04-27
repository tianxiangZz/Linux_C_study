

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <stdint.h>

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

typedef struct _pollerBase pollerBase;


#define EV_NOEVENT              (0x00)
#define EV_READABLE             (0x01)
#define EV_WRITEABLE            (0x02)
#define EV_ET                   (0x04)

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _pollerBase
{
    void *(*create)();
    void (*destroy)(void *pollable);

    int (*add)(void *pollable, int fd, uint32_t events);
    int (*mod)(void *pollable, int fd, uint32_t events);
    int (*del)(void *pollable, int fd, uint32_t events);

    int (*dispatch)(void *pollable, struct timeval *tv);
};










