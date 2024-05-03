#ifndef __BUFFER_H__
#define __BUFFER_H__


#ifdef __cplusplus
extern "C" {
#endif                      /** __cplusplus */

/*************************************************************************************************/
/**                                        INCLUDE FILES                                       **/
/*************************************************************************************************/

/*************************************************************************************************/
/**                                            TYPES                                            **/
/*************************************************************************************************/

struct _buffer;
typedef struct _buffer buffer;

/*************************************************************************************************/
/**                                       PUBLIC FUNCTIONS                                      **/
/*************************************************************************************************/

buffer *bufferCreate();
void bufferDestroy(void *data);

size_t bufferReadableSize(buffer *buf);
const void *bufferReadPeek(buffer *buf);
int bufferReadDrain(buffer *buf, size_t dataLen);
int bufferReadWithRemove(buffer *buf, void *data, size_t dataLen);
// int bufferReadWithCopyout(buffer *buf, void *data, size_t dataLen);

size_t bufferWritableSize(buffer *buf);
void *bufferWritePeek(buffer *buf);
int bufferWriteAlready(buffer *buf, size_t dataLen);
int bufferWriteUndo(buffer *buf, size_t dataLen);
int bufferWriteAppend(buffer *buf, const void *data, size_t dataLen);

// show: buffer's addr、size、readableLen
void bufferInfoShow(buffer *buf);



#ifdef __cplusplus
}
#endif                  /** __cplusplus */

#endif                  /** __BUFFER_H__ */


