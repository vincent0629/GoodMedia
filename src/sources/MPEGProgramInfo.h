#ifndef _MPEGPROGRAMINFO_H_
#define _MPEGPROGRAMINFO_H_

#include "MediaInfo.h"

typedef struct _MPEGProgramInfo : MediaInfo
{
	int nVersion;
	bool bCBR;
} MPEGProgramInfo;

#endif
