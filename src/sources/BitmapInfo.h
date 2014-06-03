#ifndef _BITMAPINFO_H_
#define _BITMAPINFO_H_

#include "MediaInfo.h"

typedef struct _BitmapInfo : VideoInfo
{
	long nCompression;
	unsigned char *pColorTable;
	int nBitsPerPixel;
	int nColorUsed;
	bool bTopDown;
} BitmapInfo;

#endif
