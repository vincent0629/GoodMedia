#ifndef _PESPACKETINFO_H_
#define _PESPACKETINFO_H_

#include "MediaInfo.h"

typedef struct _PESPacketInfo : MediaInfo
{
	int nStreamID;
	int nStreamSubID;
} PESPacketInfo;

#endif
