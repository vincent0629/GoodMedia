#ifndef _WAVINFO_H_
#define _WAVINFO_H_

#include "MediaInfo.h"

typedef struct _WAVInfo : AudioInfo
{
	long nFormatTag;
	int nBitsPerSample;
} WAVInfo;

#endif
