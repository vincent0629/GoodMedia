#ifndef _MATROSKAINFO_H_
#define _MATROSKAINFO_H_

#include "MediaInfo.h"

typedef struct _MatroskaInfo : MediaInfo
{
	int nVersion;
	int nReadVersion;
	int nTypeVersion;
	int nTypeReadVersion;
} MatroskaInfo;

#endif
