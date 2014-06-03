#ifndef _AACINFO_H_
#define _AACINFO_H_

#include "MediaInfo.h"

enum AACHeaderType {AAC_HEADER_NONE, AAC_HEADER_ADIF, AAC_HEADER_ADTS};
enum AACProfile {AAC_PROFILE_MAIN, AAC_PROFILE_LC, AAC_PROFILE_SSR, AAC_PROFILE_LTP};

typedef struct _AACInfo : AudioInfo
{
	AACHeaderType nHeaderType;
	AACProfile nProfile;
} AACInfo;

#endif
