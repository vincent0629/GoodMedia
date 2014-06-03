#ifndef _AIFFINFO_H_
#define _AIFFINFO_H_

#include "MediaInfo.h"

typedef struct _AIFFInfo : MediaInfo
{
	bool bCompressed;
	char pCompressionType[5];
	char *pCompressionName;
} AIFFInfo;

#endif
