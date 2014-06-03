#ifndef _WAVEFORMATEXTENSIBLE_H_
#define _WAVEFORMATEXTENSIBLE_H_

#include <windows.h>
#include <mmsystem.h>

typedef struct
{
	WAVEFORMATEX Format;
	union
	{
		WORD wValidBitsPerSample;
		WORD wSamplesPerBlock;
		WORD wReserved;
	} Samples;
	DWORD dwChannelMask;
	GUID SubFormat;
} WAVEFORMATEXTENSIBLE;

extern void InitWaveFormatExt(WAVEFORMATEXTENSIBLE *pFormatExt, int nSampleRate, int nBitsPerSample, int nChannel);

#endif
