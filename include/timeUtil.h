#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <sys/time.h>



/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

int timeUtilTv2Msec(const struct timeval *tv, long *result);
int timeUtilTimerAdd(const struct timeval *t1, const struct timeval *t2, struct timeval *end);
void timeUtilTimeGet(struct timeval *tv);





#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __TIME_UTIL_H__ **/