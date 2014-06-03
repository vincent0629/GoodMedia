#ifndef _THEORAINFO_H_
#define _THEORAINFO_H_

#include "MediaInfo.h"

typedef struct _TheoraInfo : VideoInfo
{
	int nMajorVersion, nMinorVersion, nVersionRevision;
	int nFrameWidth, nFrameHeight;
	int nOffsetX, nOffsetY;
	float fFrameRate;
} TheoraInfo;

#endif
