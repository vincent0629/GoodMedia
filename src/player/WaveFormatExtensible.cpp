#include "WaveFormatExtensible.h"

enum
{
	SPEAKER_FRONT_LEFT = 0x1,
	SPEAKER_FRONT_RIGHT = 0x2,
	SPEAKER_FRONT_CENTER = 0x4,
	SPEAKER_LOW_FREQUENCY = 0x8,
	SPEAKER_BACK_LEFT = 0x10,
	SPEAKER_BACK_RIGHT = 0x20,
	SPEAKER_FRONT_LEFT_OF_CENTER = 0x40,
	SPEAKER_FRONT_RIGHT_OF_CENTER = 0x80,
	SPEAKER_BACK_CENTER = 0x100,
	SPEAKER_SIDE_LEFT = 0x200,
	SPEAKER_SIDE_RIGHT = 0x400,
	SPEAKER_TOP_CENTER = 0x800,
	SPEAKER_TOP_FRONT_LEFT = 0x1000,
	SPEAKER_TOP_FRONT_CENTER = 0x2000,
	SPEAKER_TOP_FRONT_RIGHT = 0x4000,
	SPEAKER_TOP_BACK_LEFT = 0x8000,
	SPEAKER_TOP_BACK_CENTER = 0x10000,
	SPEAKER_TOP_BACK_RIGHT = 0x20000
};

void InitWaveFormatExt(WAVEFORMATEXTENSIBLE *pFormatExt, int nSampleRate, int nBitsPerSample, int nChannel)
{
	WAVEFORMATEX *pFormat;
	GUID MEDIASUBTYPE_PCM = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

	memset(pFormatExt, 0, sizeof(WAVEFORMATEXTENSIBLE));
	pFormat = &pFormatExt->Format;
	pFormat->nChannels = nChannel;
	pFormat->nSamplesPerSec = nSampleRate;
	pFormat->wBitsPerSample = nBitsPerSample;
	pFormat->nBlockAlign = pFormat->nChannels * (pFormat->wBitsPerSample >> 3);
	pFormat->nAvgBytesPerSec = pFormat->nSamplesPerSec * pFormat->nBlockAlign;
	if (nChannel <= 2 && nBitsPerSample <= 16)
		pFormat->wFormatTag = WAVE_FORMAT_PCM;
	else
	{
		pFormat->wFormatTag = 0xFFFE;  //WAVE_FORMAT_EXTENSIBLE
		pFormat->cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);  //22
		pFormatExt->Samples.wValidBitsPerSample = pFormat->wBitsPerSample;
		switch (nChannel)
		{
			case 1:
				pFormatExt->dwChannelMask = SPEAKER_FRONT_LEFT;
				break;
			case 2:
				pFormatExt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
				break;
			case 4:
				pFormatExt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
				break;
			case 6:
				pFormatExt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY;
				break;
			case 7:
				pFormatExt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_CENTER;
				break;
			case 8:
				pFormatExt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
				break;
			default:
				pFormatExt->dwChannelMask = 0;
				break;
		}
		pFormatExt->SubFormat = MEDIASUBTYPE_PCM;
	}
}
