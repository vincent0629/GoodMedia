#ifndef _VORBISINFO_H_
#define _VORBISINFO_H_

#include "MediaInfo.h"

typedef struct _VorbisInfo : AudioInfo
{
	wchar_t *pVendor;
	int nUserComment;
	wchar_t **pUserComment;
} VorbisInfo;

#endif
