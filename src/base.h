#ifndef __BASE_H__
#define __BASE_H__

#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */


/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

typedef void (*dataDestroyFunc)(void* data);

#define UNUSED(var)                               ((void) var)

#define RET_OK                      (0)
#define RET_ERROR                   (-1)
#define RET_PARA_ERROR              (-2)

#define INVALID_FD                  (-1)
#define DEFAULT_VALUE_ZERO          (0x00)
#define VAR_TO_BOOL(var)            (!!(var))

#define BASE_MALLOC(size)                   malloc(size)
#define BASE_CALLOC(n, size)                calloc((n), (size))
#define BASE_REALLOC(ptr, size)             realloc((ptr), (size))
#define BASE_FREE_SYMBOL                    free
#define BASE_FREE(ptr)              \
    do                              \
    {                               \
        if (NULL != (ptr))          \
        {                           \
            BASE_FREE_SYMBOL(ptr);  \
            ptr = NULL;             \
        }                           \
    } while (0)

#define BASE_DESTROY(ptr, destroyFunc)  \
    do                                  \
    {                                   \
        if (NULL != (ptr))              \
        {                               \
            (destroyFunc)(ptr);         \
            ptr = NULL;                 \
        }                               \
    } while (0)

#define BASE_CLOSE_FD(fd)           \
    do                              \
    {                               \
        if (INVALID_FD != (fd))     \
        {                           \
            close(fd);              \
            fd = INVALID_FD;        \
        }                           \
    } while (0)

#define CHECK_INPARA_ENSURE(exp)        \
    do                                  \
    {                                   \
        if (!(exp))                     \
        {                               \
            return RET_PARA_ERROR;      \
        }                               \
    } while (0)

#define CHECK_INPARA_ENSURE_RETNULL(exp)    \
    do                                      \
    {                                       \
        if (!(exp))                         \
        {                                   \
            return NULL;                    \
        }                                   \
    } while (0)





#ifdef __cplusplus
}
#endif                  /** __cplusplus */


#endif                  /** __BASE_H__ */