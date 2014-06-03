#ifndef _MEDIAINFO_H_
#define _MEDIAINFO_H_

#include "MediaType.h"

typedef struct _MediaInfo
{
	unsigned int nSize;
} MediaInfo;

typedef struct _TextInfo : MediaInfo
{
	char pText[256];
} TextInfo;

typedef struct _AudioInfo : MediaInfo
{
	int nSampleRate, nChannel;
} AudioInfo;

typedef struct _AudioRenderInfo : AudioInfo
{
	int nBitsPerSample;
} AudioRenderInfo;

typedef struct _VideoInfo : MediaInfo
{
	int nWidth, nHeight;
} VideoInfo;

enum PixelFormat {PIXEL_UNKNOWN, PIXEL_RGB555, PIXEL_RGB565, PIXEL_RGB24, PIXEL_RGB32, PIXEL_YUY2, PIXEL_YV12, PIXEL_FORMAT_NUM};
typedef struct _VideoRenderInfo : VideoInfo
{
	float fPixelAspectRatio;
	PixelFormat nPixelFormat;
} VideoRenderInfo;

#endif
