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

int timeUtilTimerAdd(const struct timeval *t1, const struct timeval *t2, struct timeval *end)
{
    CHECK_INPARA_ENSURE((NULL != t1) && (NULL != t2) && (NULL != end));

    end->tv_sec = t1->tv_sec + t2->tv_sec;
    end->tv_usec = t1->tv_usec + t2->tv_usec;

	if (end->tv_usec >= 1000000)
    {			
		++ (end->tv_sec);				
		end->tv_usec -= 1000000;	
    }		
    return RET_OK;
}

void timeUtilTimeGet(struct timeval *tv)
{
    if (NULL != tv)
    {
        gettimeofday(tv, NULL);
    }
    return ;
}


