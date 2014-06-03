#include "DefaultDecoderFactory.h"
#include "DecoderFactory.h"
#include "AC3Decoder.h"
#include "BitmapDecoder.h"
#include "MPEGAudioDecoder.h"
#include "MPEGVideoDecoder.h"
#include "PCMDecoder.h"
#include <stdio.h>

static CMediaDecoder *createFunc(MediaType type)
{
	CMediaDecoder *pDecoder;

	pDecoder = NULL;
	switch (type)
	{
		case MEDIA_TYPE_AC3:
			pDecoder = new CAC3Decoder();
			break;
		case MEDIA_TYPE_BITMAP:
			pDecoder = new CBitmapDecoder();
			break;
		case MEDIA_TYPE_MPEGAUDIO:
			pDecoder = new CMPEGAudioDecoder();
			break;
		case MEDIA_TYPE_MPEGVIDEO:
			pDecoder = new CMPEGVideoDecoder();
			break;
		case MEDIA_TYPE_PCM:
			pDecoder = new CPCMDecoder();
			break;
		default:
			break;
	}
	return pDecoder;
}

void CDefaultDecoderFactory::Register(void)
{
	CDecoderFactory::Register(MEDIA_TYPE_AC3, createFunc);
	CDecoderFactory::Register(MEDIA_TYPE_BITMAP, createFunc);
	CDecoderFactory::Register(MEDIA_TYPE_MPEGAUDIO, createFunc);
	CDecoderFactory::Register(MEDIA_TYPE_MPEGVIDEO, createFunc);
	CDecoderFactory::Register(MEDIA_TYPE_PCM, createFunc);
}
