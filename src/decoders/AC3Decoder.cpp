#include "AC3Decoder.h"
#include <string.h>

CAC3Decoder::CAC3Decoder()
{
}

CAC3Decoder::~CAC3Decoder()
{
}

bool CAC3Decoder::SetInputInfo(MediaType type, MediaInfo *pInfo)
{
	if (pInfo->nSize < sizeof(AudioInfo))
		return false;

	m_Info = *(AudioInfo *)pInfo;
	return true;
}

bool CAC3Decoder::GetOutputInfo(MediaInfo *pInfo)
{
	AudioRenderInfo *pAudioInfo;

	if (pInfo->nSize < sizeof(AudioRenderInfo))
		return false;

	pAudioInfo = (AudioRenderInfo *)pInfo;
	pAudioInfo->nSize = sizeof(AudioRenderInfo);
	pAudioInfo->nSampleRate = m_Info.nSampleRate;
	pAudioInfo->nBitsPerSample = sizeof(short) << 3;
	pAudioInfo->nChannel = m_Info.nChannel;
	return true;
}

int CAC3Decoder::GetInputSize(void)
{
	return 8192;
}

int CAC3Decoder::GetOutputSize(void)
{
	return 1536 * sizeof(short) * 6;
}

int CAC3Decoder::Decode(unsigned char *pInData, int nInSize, void *pOutData)
{
	int nOutSize;

	nOutSize = 1536 * sizeof(short) * m_Info.nChannel;
	memcpy(pOutData, 0, nOutSize);
	return nOutSize;
}

int CAC3Decoder::SPDIF(unsigned char *pInData, int nInSize, unsigned char *pOutData)
{
	unsigned short *pso;
	int i, nOutSize;

	pso = (unsigned short *)pOutData;
	pso[0] = 0xF872;
	pso[1] = 0x4E1F;
	pso[2] = 0x0001;
	pso[3] = nInSize << 3;
	if (*(unsigned short *)pInData == 0x770B)
	{
		for (i = 0; i < nInSize; i += 2)  //swap byte order and copy
		{
			pOutData[i + 8] = pInData[i + 1];
			pOutData[i + 9] = pInData[i];
		}
	}
	else
		memcpy(pso + 4, pInData, nInSize);
	nOutSize = 1536 * sizeof(short) * 2;
	memset(pOutData + 8 + nInSize, 0, nOutSize - 8 - nInSize);
	return nOutSize;
}
