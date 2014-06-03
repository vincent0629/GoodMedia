#ifndef _PCMINFO_H_
#define _PCMINFO_H_

#include "MediaInfo.h"

enum PCMFormat {PCM_UNKNOWN, PCM_LINEAR, PCM_ALAW, PCM_MULAW, PCM_FLOAT, PCM_FORMAT_NUM};
enum PCMByteOrder {PCM_LITTLE_ENDIAN, PCM_BIG_ENDIAN};

typedef struct _PCMInfo : AudioInfo
{
	PCMFormat nFormat;
	int nBitsPerSample;
	PCMByteOrder nByteOrder;
	bool bSigned;
} PCMInfo;

#endif
