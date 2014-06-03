#ifndef _DTSINFO_H_
#define _DTSINFO_H_

#include "MediaInfo.h"

typedef struct _DTSInfo : AudioInfo
{
	int nBitsPerSample;
} DTSInfo;

#endif
