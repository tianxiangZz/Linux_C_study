#ifndef __SOCKET_UTIL_H__
#define __SOCKET_UTIL_H__


#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/


/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

int socketfdNonBlockSet(int socketfd, int isOpen);
int socketfdCloexecSet(int socketfd, int isOpen);
int socketfdNoDelaySet(int socketfd, int isOpen);

int socketfdReuseAddrSet(int socketfd, int isOpen);
int socketfdReusePortSet(int socketfd, int isOpen);

int socketfdSendTimeoutSet(int socketfd, const struct timeval *tv);
int socketfdRecvTimeoutSet(int socketfd, const struct timeval *tv);

int socketfdKeepAliveSet(int socketfd, int isOpen);

int unixDomainSocketServerCreate(const char *unixPath);
int tcpSocketServerCreate(const char *ip, const int port);

int unixSocketConnect(const char *unixPath, const struct timeval *timeout);
int tcpSocketConnect(const char *ip, const int port, const struct timeval *timeout);


#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __SOCKET_UTIL_H__ */


