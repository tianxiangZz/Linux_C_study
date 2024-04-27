#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

int timeUtilTv2Msec(const struct timeval *tv, long *result);






#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __TIME_UTIL_H__ **/