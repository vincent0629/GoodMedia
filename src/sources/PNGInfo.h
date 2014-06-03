#ifndef _PNGINFO_H_
#define _PNGINFO_H_

#include "MediaInfo.h"

typedef enum {PNG_UNKNOWNCOLOR, PNG_GREYSCALE, PNG_TRUECOLOR, PNG_INDEXEDCOLOR, PNG_GREYSCALEWITHALPHA, PNG_TRUECOLORWITHALPHA} PNGColorType;

typedef struct _PNGInfo : VideoInfo
{
	unsigned char *pColorTable;
	int nBitsPerPixel;
	PNGColorType nColorType;
	int nColorUsed;
} PNGInfo;

#endif
