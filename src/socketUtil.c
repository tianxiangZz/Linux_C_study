
/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include "fmacros.h"
#include <string.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>


#include "socketUtil.h"
#include "timeUtil.h"
#include "base.h"
#include "log.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define SOCKET_CONNECT_RETRIES          (3)
#define SOCKET_KEEPALIVE_INTERVAL       (15)
#define SOCKT_LISTEN_BACKLOG            (511)
#define UNIX_SOCKET_FILE_PERM           (700)

#define CHECK_PORT_IS_LEGAL(port)           ((port) > 0 && (port) <= 65535)  

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                       FUNCTION DECLARATION                                      **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                            VARIABLES                                       **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static int socketfdCheckConnectDone(int socketfd, struct sockaddr *saddr, socklen_t addrlen, int *completed)
{
    int ret = 0;
    
    CHECK_INPARA_ENSURE((INVALID_FD != socketfd) && (NULL != saddr));

    if (0 == connect(socketfd, saddr, addrlen))
    {
        *completed = 1;
        return RET_OK;
    }

    switch (errno)
    {
        case EISCONN:
            *completed = 1;
            return RET_OK;
        case EALREADY:
        case EINPROGRESS:
        case EWOULDBLOCK:
            *completed = 0;
            return RET_OK;
        default:
            return RET_ERROR;
    }

    return RET_OK;
}

static int socketfdConnectWaitReady(int socketfd, struct sockaddr *saddr, socklen_t addrlen, long msec)
{
    int ret = 0;
    struct pollfd   wfd[1] = { 0 };

    wfd[0].fd     = socketfd;
    wfd[0].events = POLLOUT;

    if (errno == EINPROGRESS) 
    {
        if (-1 == (ret = poll(wfd, 1, msec))) 
        {
            DEBUG_LOG_ERROR("poll error: %s", strerror(errno));
            return RET_ERROR;
        } 
        else if (ret == 0) 
        {
            errno = ETIMEDOUT;
            DEBUG_LOG_ERROR("connect timeout!");
            return RET_ERROR;
        }

        if (RET_OK != socketfdCheckConnectDone(socketfd, saddr, addrlen, &ret) || 
            ret == 0)
        {
            DEBUG_LOG_ERROR("connect error!");
            return RET_ERROR;
        }

        return RET_OK;
    }

    return RET_ERROR;
}

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

int unixDomainSocketServerCreate(const char *unixPath)
{
    int socketfd = INVALID_FD;
    struct sockaddr_un sa = { 0 };

    CHECK_INPARA_ENSURE((NULL != unixPath));

    if (strlen(unixPath) > (sizeof(sa.sun_path) - 1)) 
    {
        DEBUG_LOG_ERROR("unix socket path too long (%zu), must be less than %zu", strlen(unixPath), sizeof(sa.sun_path));
        return RET_ERROR;
    }

    unlink(unixPath);

    /** AF_UNIX = AF_LOCAL、SOCK_STREAM or SOCK_SEQPACKET  **/
    if (INVALID_FD == (socketfd = socket(AF_UNIX, SOCK_STREAM, 0))) 
    {
        DEBUG_LOG_ERROR("socket error: %s", strerror(errno));
        return RET_ERROR;
    }

    if (RET_ERROR == socketfdReuseAddrSet(socketfd, SWITCH_ON)) 
    {
        BASE_CLOSE_FD(socketfd);
        return RET_ERROR;
    }

    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, unixPath, sizeof(sa.sun_path) - 1);

    if (0 != bind(socketfd, (struct sockaddr *)&sa, SUN_LEN(&sa))) 
    {
        BASE_CLOSE_FD(socketfd);
        DEBUG_LOG_ERROR("bind error: %s", strerror(errno));
        return RET_ERROR;
    }

    if (0 != listen(socketfd, SOCKT_LISTEN_BACKLOG)) 
    {
        BASE_CLOSE_FD(socketfd);
        DEBUG_LOG_ERROR("listen error: %s", strerror(errno));
        return RET_ERROR;
    }

    chmod(sa.sun_path, UNIX_SOCKET_FILE_PERM);

    if (RET_OK != socketfdCloexecSet(socketfd, SWITCH_ON) ||
        RET_OK != socketfdNonBlockSet(socketfd, SWITCH_ON))
    {
        BASE_CLOSE_FD(socketfd);
        return RET_ERROR;
    }

    return socketfd;
}

int tcpSocketServerCreate(const char *ip, const int port)
{
    int socketfd = INVALID_FD, rv;
    char _port[6] = { 0 };          /* strlen("65535") */
    struct addrinfo hints = { 0 }, *servinfo = NULL, *p = NULL;

    CHECK_INPARA_ENSURE((NULL != ip) && CHECK_PORT_IS_LEGAL(port));

    snprintf(_port, 6, "%d", port);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */

    if (0 != (rv = getaddrinfo(ip, _port, &hints, &servinfo))) 
    {
        DEBUG_LOG_ERROR("tcpSocketServerCreate getaddrinfo error: %s", gai_strerror(rv));
        return RET_ERROR;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
        if (INVALID_FD == (socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)))
        {
            continue;
        }

        /** maybe add reuse port **/
        if (RET_ERROR == socketfdReuseAddrSet(socketfd, SWITCH_ON))
        {
            goto error;
        }

        if (0 != bind(socketfd, (struct sockaddr *)&sa, SUN_LEN(&sa))) 
        {
            DEBUG_LOG_ERROR("bind error: %s", strerror(errno));
            goto error;
        }

        if (0 != listen(socketfd, SOCKT_LISTEN_BACKLOG)) 
        {
            DEBUG_LOG_ERROR("listen error: %s", strerror(errno));
            goto error;
        }

        if (RET_OK != socketfdCloexecSet(socketfd, SWITCH_ON) ||
            RET_OK != socketfdNonBlockSet(socketfd, SWITCH_ON))
        {
            goto error;
        }

        BASE_DESTROY(servinfo, freeaddrinfo);
        return socketfd;
    }

    if (NULL == p) 
    {
        DEBUG_LOG_ERROR("unable to bind socket, errno: %s", strerror(errno));
    }

error:
    BASE_CLOSE_FD(socketfd);
    BASE_DESTROY(servinfo, freeaddrinfo);
    return RET_ERROR;
}

int unixSocketConnect(const char *unixPath, const struct timeval *connectTimeout)
{
    int socketfd = INVALID_FD;
	long timeoutMsec = -1;
	struct sockaddr_un sa = { 0 };

    CHECK_INPARA_ENSURE((NULL != unixPath) && (strlen(unixPath) > (sizeof(sa.sun_path) - 1)));

    if (RET_OK != timeUtilTv2Msec(connectTimeout, &timeoutMsec))
	{
        DEBUG_LOG_ERROR("connectTimeout error");
		return RET_ERROR;
	}

    if (INVALID_FD == (socketfd = socket(AF_UNIX, SOCK_STREAM, 0)))
    {
        DEBUG_LOG_ERROR();
        return RET_ERROR;
    }

	if (RET_OK != socketfdNonBlockSet(socketfd, SWITCH_ON))
	{
		goto error;
	}

    sa->sun_family = AF_UNIX;
    strncpy(sa->sun_path, unixPath, sizeof(sa->sun_path) - 1);

	if (-1 == connect(socketfd, (struct sockaddr*)&sa, sizeof(struct sockaddr_un))) 
	{
        if (errno == EINPROGRESS) 
        {
            if (RET_OK != socketfdConnectWaitReady(socketfd, (struct sockaddr*)&sa,  timeoutMsec))
			{
				goto error;
			} 
        } 
		else 
		{
            goto error;
        }
    }

	return socketfd;
error:
    BASE_CLOSE_FD(socketfd);
    return RET_ERROR;
}

int tcpSocketConnect(const char *ip, const int port, const struct timeval *connectTimeout)
{
    int socketfd = INVALID_FD, rv = 0, reuses = 0;
    char _port[6];              /* strlen("65535"); */
    struct addrinfo hints = { 0 }, *servinfo = NULL, *bservinfo = NULL, *p = NULL , *b = NULL;
    long timeoutMsec = -1;

    CHECK_INPARA_ENSURE((NULL != ip) && CHECK_PORT_IS_LEGAL(port));

    if (RET_OK != timeUtilTv2Msec(connectTimeout, &timeoutMsec))
	{
        DEBUG_LOG_ERROR("connectTimeout error");
		return RET_ERROR;
	}

    snprintf(_port, 6, "%d", port);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (0 != (rv = getaddrinfo(ip, _port, &hints, &servinfo))) 
    {
        return RET_ERROR;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
addrretry:
        if (INVALID_FD == (socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)))
        {    
            continue;
        }

	    if (RET_OK != socketfdNonBlockSet(socketfd, SWITCH_ON))
	    {
		    goto error;
	    }

        if (-1 == connect(socketfd, p->ai_addr, p->ai_addrlen)) 
        {
            if (errno == EHOSTUNREACH) 
            {
                BASE_CLOSE_FD(socketfd);
                continue;
            } 
            else if (errno == EINPROGRESS) 
            {
                if (RET_OK != socketfdConnectWaitReady(socketfd, p->ai_addr, p->ai_addrlen, timeoutMsec))
                {
                    goto error;
                }
            }
            else if (errno == EADDRNOTAVAIL) 
            {
                if ((++ reuses) >= SOCKET_CONNECT_RETRIES) 
                {
                    goto error;
                }
                else
                {
                    BASE_CLOSE_FD(socketfd);
                    goto addrretry;
                }
            } 
            else 
            {
                goto error;
            }
        }

        BASE_DESTROY(servinfo, freeaddrinfo);
        return socketfd;
    }
    
    if (p == NULL)
    {
        DEBUG_LOG_ERROR("Can't create tcp socket connect: %s",strerror(errno));
        goto error;
    }

error:
    BASE_CLOSE_FD(socketfd);
    BASE_DESTROY(servinfo, freeaddrinfo);
    return RET_ERROR;
}

int socketfdReuseAddrSet(int socketfd, int isOpen)
{
    int yes = VAR_TO_BOOL(isOpen);

    CHECK_INPARA_ENSURE(INVALID_FD != socketfd);
  
    if (-1 == setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) 
    {
        DEBUG_LOG_ERROR("setsockopt SO_REUSEADDR: %s", strerror(errno));
        return RET_ERROR;
    }

    return RET_OK;
}

int socketfdReusePortSet(int socketfd, int isOpen)
{
    int yes = VAR_TO_BOOL(isOpen);

    CHECK_INPARA_ENSURE(INVALID_FD != socketfd);
  
    if (-1 == setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes))) 
    {
        DEBUG_LOG_ERROR("setsockopt SO_REUSEPORT: %s", strerror(errno));
        return RET_ERROR;
    }

    return RET_OK;    
}

int socketfdSendTimeoutSet(int socketfd, const struct timeval *tv)
{
    CHECK_INPARA_ENSURE((INVALID_FD != socketfd) && (NULL != tv));

    if (-1 == setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, tv, sizeof(struct timeval))) 
    {
        DEBUG_LOG_ERROR("setsockopt SO_SNDTIMEO: %s", strerror(errno));
        return RET_ERROR;
    }

    return RET_OK;
}

int socketfdRecvTimeoutSet(int socketfd, const struct timeval *tv)
{
    CHECK_INPARA_ENSURE((INVALID_FD != socketfd) && (NULL != tv));
    
    if (-1 == setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof(struct timeval))) 
    {
        DEBUG_LOG_ERROR("setsockopt SO_RCVTIMEO: %s", strerror(errno));
        return RET_ERROR;
    }

    return RET_OK;
}

int socketfdNonBlockSet(int socketfd, int isOpen)
{
	int flags = 0, rv = 0;

	CHECK_INPARA_ENSURE(INVALID_FD != socketfd);

    do {
        flags = fcntl(socketfd, F_GETFD);
    } while (-1 == flags && errno == EINTR);

    if (-1 == flags)
    {
        DEBUG_LOG_ERROR("fcntl(F_GETFL): %s", strerror(errno));
        return RET_ERROR;
    }

	if (VAR_TO_BOOL(isOpen) && !(flags & O_NONBLOCK))
	{
		flags |= O_NONBLOCK;
	}
	else if (!VAR_TO_BOOL(isOpen) && (flags & O_NONBLOCK))
	{
		flags &= ~O_NONBLOCK;		
	}

    do {
        rv = fcntl(socketfd, F_SETFD, flags);
    } while (-1 == rv && errno == EINTR);

    return (-1 == rv) ? RET_ERROR : RET_OK;
}

int socketfdCloexecSet(int socketfd, int isOpen)
{
    int rv = 0, flags = 0;

    CHECK_INPARA_ENSURE(INVALID_FD != socketfd);

    do {
        flags = fcntl(socketfd, F_GETFD);
    } while (-1 == flags && errno == EINTR);

    if (-1 == flags)
    {
        DEBUG_LOG_ERROR("fcntl(F_GETFL): %s", strerror(errno));
        return RET_ERROR;
    }

    if (VAR_TO_BOOL(isOpen) && !(flags & FD_CLOEXEC))
    {
        flags |= FD_CLOEXEC;
    }
    else if (!VAR_TO_BOOL(isOpen) && (flags & O_NONBLOCK))
    {
        flags &= ~O_NONBLOCK;
    }

    do {
        rv = fcntl(socketfd, F_SETFD, flags);
    } while (-1 == rv && errno == EINTR);

    return (-1 == rv) ? RET_ERROR : RET_OK;
}

int socketfdNoDelaySet(int socketfd, int isOpen)
{
    int yes = VAR_TO_BOOL(isOpen);

    CHECK_INPARA_ENSURE(INVALID_FD != socketfd);
  
    if (-1 == setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes))) 
    {
        DEBUG_LOG_ERROR("setsockopt TCP_NODELAY: %s", strerror(errno));
        return RET_ERROR;
    }

    return RET_OK;
}

int socketfdKeepAliveSet(int socketfd, int isOpen, int interval)
{
    int yes = VAR_TO_BOOL(isOpen), val = 0;

    CHECK_INPARA_ENSURE(INVALID_FD != socketfd);
  
    if (-1 == setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes))) 
    {
        DEBUG_LOG_ERROR("setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return RET_ERROR;
    }

    if (yes)
    {
        /** 
         * Default settings are more or less garbage, with the keepalive time
         * set to 7200 by default on Linux. Modify settings to make the feature
         * actually useful. 
         **/

        /* Send first probe after interval. */
        val = interval;
        if (-1 == setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val))) 
        {
            DEBUG_LOG_ERROR("setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
            return RET_ERROR;
        }

        /**
         * Send next probes after the specified interval. Note that we set the
         * delay as interval / 3, as we send three probes before detecting
         * an error (see the next setsockopt call). 
         **/
        val = interval / 3;
        if (val == 0)
        {
            val = 1;
        }
        if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)))
        {
            DEBUG_LOG_ERROR("setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
            return RET_ERROR;
        }

        /* Consider the socket in error state after three we send three ACK
         * probes without getting a reply. */
        val = 3;
        if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)))
        {
            DEBUG_LOG_ERROR("setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
            return RET_ERROR;
        }
    }

    return RET_OK;    
}

