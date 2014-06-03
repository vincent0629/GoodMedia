#ifndef _DATAQUEUE_H_
#define _DATAQUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct _DATAQUEUE;
typedef struct _DATAQUEUE DATAQUEUE;

DATAQUEUE *queueCreate(int nSize);
void queueDestroy(DATAQUEUE *pQueue);
void queuePush(DATAQUEUE *pQueue, void *pData, int nSize);
int queuePop(DATAQUEUE *pQueue, void *pData, int nSize);
int queueAvailable(DATAQUEUE *pQueue);
void queueReset(DATAQUEUE *pQueue);

#ifdef __cplusplus
}
#endif

#endif
