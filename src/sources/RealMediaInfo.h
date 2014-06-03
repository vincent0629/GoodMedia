#ifndef _REALMEDIAINFO_H_
#define _REALMEDIAINFO_H_

#include "MediaInfo.h"

typedef struct _RealMediaInfo : MediaInfo
{
	char *pTitle;
	char *pAuthor;
	char *pCopyright;
	char *pComment;
} RealMediaInfo;

#endif
