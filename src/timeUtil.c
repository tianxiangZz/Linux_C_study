/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <limits.h>

#include "timeUtil.h"
#include "base.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define MAX_SECONDS_IN_MSEC_LONG    (((LONG_MAX) - 999) / 1000)

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/


int timeUtilTv2Msec(const struct timeval *tv, long *result)
{
    long msec = -1;

    if (NULL != tv)
	{
        if (timeout->tv_usec > 1000000 || timeout->tv_sec > MAX_SECONDS_IN_MSEC_LONG) 
		{
            *result = msec;
            return RET_ERROR;
        }

        msec = (timeout->tv_sec * 1000) + ((timeout->tv_usec + 999) / 1000);

        if (msec < 0 || msec > INT_MAX) 
		{
            msec = INT_MAX;
        }
    }

    *result = msec;
    return RET_OK;
}

