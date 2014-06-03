#ifndef _FLACINFO_H_
#define _FLACINFO_H_

#include "MediaInfo.h"

typedef struct _FLACInfo : AudioInfo
{
	int nBitsPerSample;
} FLACInfo;

#endif
