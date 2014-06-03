#ifndef _QUICKTIMEINFO_H_
#define _QUICKTIMEINFO_H_

#include "MediaInfo.h"

typedef struct _QuickTimeInfo : MediaInfo
{
	char pFileType[5];
} QuickTimeInfo;

#endif
