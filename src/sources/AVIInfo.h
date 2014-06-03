#ifndef _AVIINFO_H_
#define _AVIINFO_H_

#include "MediaInfo.h"

typedef struct _AVIInfo : MediaInfo
{
	char pVideoFCC[5];
	float fFrameRate;
	char pAudioFCC[5];
} AVIInfo;

#endif
