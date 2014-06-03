#ifndef _ASFINFO_H_
#define _ASFINFO_H_

#include "MediaInfo.h"
#include <wchar.h>

typedef struct _ASFInfo : MediaInfo
{
	wchar_t *pTitle;
	wchar_t *pAuthor;
	wchar_t *pCopyright;
	wchar_t *pDescription;
	wchar_t *pRating;
	wchar_t *pAlbum;
	wchar_t *pYear;
	wchar_t *pGenre;
	bool bDRM;
	char *pKeyID;
	char *pLicenseURL;
} ASFInfo;

#endif
