#include "dataqueue.h"
#include <stdlib.h>
#include <string.h>

struct _DATAQUEUE
{
	unsigned char *pBuffer, *pHead, *pTail;
	int nSize, nAvailable;
};

DATAQUEUE *queueCreate(int nSize)
{
	DATAQUEUE *pQueue;

	pQueue = (DATAQUEUE *)malloc(sizeof(DATAQUEUE));
	pQueue->nSize = nSize;
	pQueue->pBuffer = (unsigned char *)malloc(nSize);
	queueReset(pQueue);
	return pQueue;
}

void queueDestroy(DATAQUEUE *pQueue)
{
	free(pQueue->pBuffer);
	free(pQueue);
}

void queuePush(DATAQUEUE *pQueue, void *pData, int nSize)
{
	int n;

	if (pQueue->nAvailable + nSize > pQueue->nSize)
		nSize = pQueue->nSize - pQueue->nAvailable;
	pQueue->nAvailable += nSize;

	n = pQueue->pBuffer + pQueue->nSize - pQueue->pTail;
	if (n < nSize)
	{
		memcpy(pQueue->pTail, pData, n);
		pQueue->pTail = pQueue->pBuffer;
		pData = (char *)pData + n;
		nSize -= n;
	}
	memcpy(pQueue->pTail, pData, nSize);
	pQueue->pTail += nSize;
}

int queuePop(DATAQUEUE *pQueue, void *pData, int nSize)
{
	int n, nResult;

	if (nSize > pQueue->nAvailable)
		nSize = pQueue->nAvailable;
	pQueue->nAvailable -= nSize;
	nResult = nSize;

	n = pQueue->pBuffer + pQueue->nSize - pQueue->pHead;
	if (n < nSize)
	{
		if (pData != NULL)
		{
			memcpy(pData, pQueue->pHead, n);
			pData = (char *)pData + n;
		}
		pQueue->pHead = pQueue->pBuffer;
		nSize -= n;
	}
	if (pData != NULL)
		memcpy(pData, pQueue->pHead, nSize);
	pQueue->pHead += nSize;
	return nResult;
}

int queueAvailable(DATAQUEUE *pQueue)
{
	return pQueue->nAvailable;
}

void queueReset(DATAQUEUE *pQueue)
{
	pQueue->pHead = pQueue->pTail = pQueue->pBuffer;
	pQueue->nAvailable = 0;
}
