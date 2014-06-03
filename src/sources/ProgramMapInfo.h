#ifndef _PROGRAMMAPINFO_H_
#define _PROGRAMMAPINFO_H_

#include "MediaInfo.h"

typedef struct _StreamInfo
{
	int nType;
	int nPID;
} StreamInfo;

typedef struct _ProgramMapInfo : MediaInfo
{
	int nProgramNumber;
	int nVersion;
	int nStreamNum;
	StreamInfo *pInfo;
} ProgramMapInfo;

#endif
