#include "PCMDecoder.h"
#include <string.h>

CPCMDecoder::CPCMDecoder()
{
}

CPCMDecoder::~CPCMDecoder()
{
}

bool CPCMDecoder::SetInputInfo(MediaType type, MediaInfo *pInfo)
{
	if (pInfo->nSize < sizeof(PCMInfo))
		return false;

	m_Info = *(PCMInfo *)pInfo;
	if (m_Info.nFormat != 0 && m_Info.nFormat != 1 && m_Info.nFormat != 3 && m_Info.nFormat != 6 && m_Info.nFormat != 7)  //not support
		return false;

	m_nBpsIn = (m_Info.nBitsPerSample + 7) & ~7;
	m_bSwapOrder = m_Info.nByteOrder == PCM_BIG_ENDIAN;
	return true;
}

bool CPCMDecoder::GetOutputInfo(MediaInfo *pInfo)
{
	AudioRenderInfo *pAudioInfo;

	if (pInfo->nSize < sizeof(AudioRenderInfo))
		return false;

	m_nBpsOut = (m_Info.nFormat == 3 || m_Info.nFormat == 6 || m_Info.nFormat == 7)? sizeof(short) << 3 : m_nBpsIn;
	pAudioInfo = (AudioRenderInfo *)pInfo;
	pAudioInfo->nSize = sizeof(AudioRenderInfo);
	pAudioInfo->nSampleRate = m_Info.nSampleRate;
	pAudioInfo->nBitsPerSample = m_nBpsOut;
	pAudioInfo->nChannel = m_Info.nChannel;
	return true;
}

int CPCMDecoder::GetInputSize(void)
{
	return m_Info.nSampleRate / 10 * (m_nBpsIn >> 3) * m_Info.nChannel;
}

int CPCMDecoder::GetOutputSize(void)
{
	return m_Info.nSampleRate / 10 * (m_nBpsOut >> 3) * m_Info.nChannel;
}

int CPCMDecoder::Decode(unsigned char *pInData, int nInSize, void *pOutData)
{
	static int exp_lut[8] = {0, 132, 396, 924, 1980, 4092, 8316, 16764};
	int i, j, nOutSize, nBPS;
	unsigned char *pcSrc, *pcDst;
	float *pfSrc;
	double *pdSrc;
	short *psDst;

	nOutSize = 0;
	nBPS = m_nBpsIn >> 3;
	switch (m_Info.nFormat)
	{
		case PCM_LINEAR:  //PCM
			if (!m_bSwapOrder)
				memcpy(pOutData, pInData, nInSize);
			else
			{
				pcDst = (unsigned char *)pOutData;
				for (i = 0; i < nInSize; i += nBPS)
					for (j = 0; j < nBPS; j++)
						pcDst[i + j] = pInData[i + nBPS - j - 1];
			}

			if (m_nBpsIn == 8 && m_Info.bSigned)
			{
				pcDst = (unsigned char *)pOutData;
				for (i = 0; i < nInSize; i++)
					pcDst[i] -= 0x80;
			}
			nOutSize = nInSize;
			break;
		case PCM_FLOAT:  //IEEE float
			psDst = (short *)pOutData;
			if (m_Info.nBitsPerSample == 32)
			{
				pfSrc = (float *)pInData;
				for (i = (nInSize >> 2) - 1; i >= 0; i--)
					psDst[i] = pfSrc[i] * 0x7FFF;
				nOutSize = nInSize >> 1;
			}
			else if (m_Info.nBitsPerSample == 64)
			{
				pdSrc = (double *)pInData;
				for (i = (nInSize >> 3) - 1; i >= 0; i--)
					psDst[i] = pdSrc[i] * 0x7FFF;
				nOutSize = nInSize >> 2;
			}
			break;
		case PCM_ALAW:  //ALAW
			pcSrc = (unsigned char *)pInData;
			psDst = (short *)pOutData;
			for (i = 0; i < nInSize; i++)
			{
				*pcSrc ^= 0x55;
				*psDst = (*pcSrc & 0x0F) << 4;
				j = (*pcSrc & 0x70) >> 4;
				if (j)
					*psDst = (*psDst + 0x100) << (j - 1);
				if ((*pcSrc & 0x80) == 0)
					*psDst = -*psDst;

				++pcSrc;
				++psDst;
			}
			nOutSize = nInSize << 1;
			break;
		case PCM_MULAW:  //MULAW
			pcSrc = (unsigned char *)pInData;
			psDst = (short *)pOutData;
			for (i = 0; i < nInSize; i++)
			{
				*pcSrc = ~*pcSrc;
				j = (*pcSrc >> 4) & 0x07;
				*psDst = exp_lut[j] + ((*pcSrc & 0x0F) << (j + 3));
				if (*pcSrc & 0x80)
					*psDst = -*psDst;

				++pcSrc;
				++psDst;
			}
			nOutSize = nInSize << 1;
			break;
		default:
			break;
	}
	return nOutSize;
}
