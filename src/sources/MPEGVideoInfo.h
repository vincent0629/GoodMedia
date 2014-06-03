#ifndef _MPEGVIDEOINFO_H_
#define _MPEGVIDEOINFO_H_

#include "MediaInfo.h"

typedef struct _MPEGVideoInfo : VideoInfo
{
	int nVersion;
	float fPelAspectRatio;
	int nDispAspectRatio;
	float fFrameRate;
} MPEGVideoInfo;

#endif
