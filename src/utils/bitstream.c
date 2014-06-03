#include "bitstream.h"
#include <stdlib.h>
#include <assert.h>

struct _BITSTREAM
{
	BSFUNC pFunc;
	void *pData;
	int nByte;
	int nBitIndex;
};

BITSTREAM *bsopen(BSFUNC pFunc, void *pData)
{
	BITSTREAM *pStream;

	pStream = (BITSTREAM *)malloc(sizeof(BITSTREAM));
	pStream->pFunc = pFunc;
	pStream->pData = pData;
	pStream->nByte = pStream->pFunc(pStream->pData);
	pStream->nBitIndex = 0;
	return pStream;
}

int bsread(BITSTREAM *pStream, int nBits)
{
	int nResult;
	int n;
	unsigned int nMask;

	assert(nBits < sizeof(int) * 8);

	nResult = 0;
	while (nBits > 0)
	{
		n = 8 - pStream->nBitIndex;
		if (n > nBits)
			n = nBits;
		nMask = (1 << n) - 1;
		nResult = (nResult << n) | ((pStream->nByte >> (8 - pStream->nBitIndex - n)) & nMask);
		nBits -= n;
		pStream->nBitIndex += n;
		if (pStream->nBitIndex >= 8)
		{
			pStream->nBitIndex = 0;
			pStream->nByte = pStream->pFunc(pStream->pData);
			if (pStream->nByte == -1)
				return -1;
		}
	}

	return nResult;
}

void bsseek(BITSTREAM *pStream, int nBits)
{
	int i;

	if (nBits >= 8)
	{
		for (i = nBits >> 3; i > 0; i--)
			pStream->nByte = pStream->pFunc(pStream->pData);
		nBits &= 7;
	}
	pStream->nBitIndex += nBits;
	if (pStream->nBitIndex >= 8)
	{
		pStream->nByte = pStream->pFunc(pStream->pData);
		pStream->nBitIndex -= 8;
	}
}

void bsalign(BITSTREAM *pStream)
{
	if (pStream->nBitIndex != 0)
	{
		pStream->nBitIndex = 0;
		pStream->nByte = pStream->pFunc(pStream->pData);
	}
}

void bsclose(BITSTREAM *pStream)
{
	if (pStream != NULL)
		free(pStream);
}

#if 0
int main(int argc, char *argv[])
{
	unsigned char pData[] = {0xCD, 0x0A, 0x30, 0x70};
	BITSTREAM *pStream;
	int i, n;

	pStream = bsopen(pData, 4);
	for (i = 1; i <= 7; i++)
	{
		n = bsread(pStream, i);
		assert(n == i);
		printf("%d ", n);
	}
	printf("\n");
	bsclose(pStream);
	return 0;
}
#endif
