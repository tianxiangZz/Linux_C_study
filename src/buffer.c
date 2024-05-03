
/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "base.h"
#include "log.h"

/*************************************************************************************************/
/**                                            DEFINES                                            **/
/*************************************************************************************************/

#define BUFFER_DEFAULT_BUFFER_LEN                   (1024)
#define BUFFER_DEFAULT_MAX_BUFFER_LEN               (1024 * 16)


#define BUFFER_READABLE_SIZE(buf)               (buf->writeIndex - buf->readIndex)
#define BUFFER_WRITABLE_SIZE(buf)               (buf->bufsize - buf->writeIndex)

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _buffer
{
    size_t readIndex;
    size_t writeIndex;

    size_t bufsize;
    char *buf;
};

/*************************************************************************************************/
/**                                       FUNCTION DECLARATION                                      **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                            VARIABLES                                       **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                       PRIVATE FUNCTIONS                                      **/
/*************************************************************************************************/

static int bufferReadableToBegin(buffer *buf)
{
    size_t readableLen = 0;
    CHECK_INPARA_ENSURE(NULL != buf);

    readableLen = BUFFER_READABLE_SIZE(buf);
    memmove(buf->buf, (buf->buf + buf->readIndex), readableLen);
    buf->readIndex = 0;
    buf->writeIndex = buf->readIndex + readableLen;

    return RET_OK;
}

static int bufferEnsureWritableSize(buffer *buf, size_t dataLen)
{
    size_t writableLen = 0, readableLen = 0;
    size_t newLen = 0, minLen = 0;
    void *newbuf = NULL;

    CHECK_INPARA_ENSURE(NULL != buf);

    writableLen = BUFFER_WRITABLE_SIZE(buf);

    if (dataLen <= writableLen)
    {
        return RET_OK;
    }
    else if (dataLen <= (writableLen + buf->readIndex))
    {
        // move
        return bufferReadableToBegin(buf);
    }
    else
    {
        // move and realloc
        if (RET_OK != bufferReadableToBegin(buf))
        {
            return RET_ERROR;
        }
        minLen = dataLen + BUFFER_READABLE_SIZE(buf);
        newLen = buf->bufsize;
        while (newLen < minLen)
        {
            newLen <<= 1;
        }
        if (newLen > BUFFER_DEFAULT_MAX_BUFFER_LEN)
        {
            newLen = minLen;
        }

        if (NULL == (newbuf = BASE_REALLOC(buf->buf, newLen)))
        {
            return RET_ERROR;
        }

        buf->bufsize = newLen;
        buf->buf = newbuf;
        return RET_OK;
    }

    return RET_ERROR;
}

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

buffer *bufferCreate()
{
    buffer *this = NULL;

    if (NULL == (this = BASE_CALLOC(1, sizeof(buffer))))
    {
        DEBUG_LOG_ERROR("buffer create error: oom!");
        return NULL;
    }

    if (NULL == (this->buf = BASE_MALLOC(BUFFER_DEFAULT_BUFFER_LEN)))
    {
        DEBUG_LOG_ERROR("buffer's buf malloc error: oom!");
        goto error;
    }

    this->bufsize = BUFFER_DEFAULT_BUFFER_LEN;
    this->readIndex = this->writeIndex = 0;

    return this;
error:
    bufferDestroy(this);
    return NULL;
}

void bufferDestroy(void *data)
{
    buffer *this = (buffer *)data;
    if (NULL != this)
    {
        BASE_FREE(this->buf);
        BASE_FREE(this);
    }
    return ;
}

size_t bufferReadableSize(buffer *buf)
{
    CHECK_INPARA_ENSURE(NULL != buf);

    return BUFFER_READABLE_SIZE(buf);
}

const void *bufferReadPeek(buffer *buf)
{
    CHECK_INPARA_ENSURE_RETNULL(NULL != buf && NULL != buf->buf);

    return (const void *)(buf->buf + buf->readIndex);
}

int bufferReadDrain(buffer *buf, size_t dataLen)
{
    CHECK_INPARA_ENSURE(NULL != buf);

    if (dataLen < BUFFER_READABLE_SIZE(buf))
    {
        buf->readIndex += dataLen;
    }
    else
    {
        buf->readIndex = buf->writeIndex = 0;
    }

    return RET_OK;
}

int bufferReadWithRemove(buffer *buf, void *data, size_t dataLen)
{
    size_t readableLen = 0;

    CHECK_INPARA_ENSURE(NULL != buf && NULL != buf->buf && NULL != data && 0 >= dataLen);

    readableLen = BUFFER_READABLE_SIZE(buf);

    if (dataLen < readableLen)
    {
        memcpy(data, (buf->buf + buf->readIndex), dataLen);
        buf->readIndex += dataLen;
        return dataLen;
    }
    else            // dataLen >= readableLen
    {
        memcpy(data, (buf->buf + buf->readIndex), readableLen);
        buf->readIndex = buf->writeIndex = 0;
        return readableLen;
    }

    return RET_ERROR;   
}
// int bufferReadWithCopyout(buffer *buf, void *data, size_t dataLen);

size_t bufferWritableSize(buffer *buf)
{
    CHECK_INPARA_ENSURE(NULL != buf);

    return BUFFER_WRITABLE_SIZE(buf);
}

void *bufferWritePeek(buffer *buf)
{
    CHECK_INPARA_ENSURE_RETNULL(NULL != buf && NULL != buf->buf);

    return (void *)(buf->buf + buf->writeIndex);
}

int bufferWriteAlready(buffer *buf, size_t dataLen)
{
    CHECK_INPARA_ENSURE(NULL != buf);

    if (dataLen <= BUFFER_WRITABLE_SIZE(buf))
    {
        buf->writeIndex += dataLen;
    }
    else
    {
        return RET_ERROR;
    }

    return RET_OK;
}

void bufferWriteUndo(buffer *buf, size_t dataLen)
{
    size_t readableLen = 0;
    if (NULL == buf || 0 == dataLen)
    {
        return ;
    }

    readableLen = BUFFER_READABLE_SIZE(buf);
    if (dataLen < readableLen)
    {
        buf->writeIndex -= dataLen;
    }
    else if (dataLen == readableLen)
    {
        buf->readIndex = buf->writeIndex = 0;
    }
    else
    {
        return ;
    }

    return ;
}

int bufferWriteAppend(buffer *buf, const void *data, size_t dataLen)
{
    CHECK_INPARA_ENSURE(NULL != buf && NULL != buf->buf && NULL != data && 0 >= dataLen);

    if (RET_OK != bufferEnsureWritableSize(buf, dataLen))
    {
        return RET_ERROR;
    }

    memcpy(buf->buf + buf->writeIndex, data, dataLen);

    buf->writeIndex += dataLen;
    return RET_OK;
}








