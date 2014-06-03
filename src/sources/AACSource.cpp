#include "AACSource.h"
#include <string.h>
#include <stdint.h>

static int sampleRateTable[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, -1, -1, -1};

CAACSource::CAACSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[5];
	int i, j, nValue, nBitStreamType, nNumProgramConfigElements;
	int nFramePos[4], nFrameSize[4];
	bool bDone;
	
	str[4] = '\0';
	SourceReadData(str, 4);
	if (strcmp(str, "ADIF") == 0)
	{
		nValue = SourceReadData();
		if (nValue & 0x80)  //copyright id present
		{
			SourceSeekData(8, SEEK_CUR);
			nValue = SourceReadData();
		}
		nBitStreamType = (nValue >> 4) & 1;
		SourceSeekData(2, SEEK_CUR);
		nValue = SourceReadData();
		nNumProgramConfigElements = (nValue >> 1) & 0x0F;
		//for (i = 0; i <= nNumProgramConfigElements; i++)
		{
			if (nBitStreamType == 0)
			{
				SourceSeekData(2, SEEK_CUR);
				nValue = SourceReadBigEndian(2);
				m_AACInfo.nSampleRate = sampleRateTable[(nValue >> 3) & 0x0F];
			}
			else
			{
				nValue = SourceReadBigEndian(2);
				m_AACInfo.nSampleRate = sampleRateTable[(nValue >> 7) & 0x0F];
			}
		}
		m_AACInfo.nChannel = 0;  //
		m_nOutputNum = 1;
		m_AACInfo.nHeaderType = AAC_HEADER_ADIF;
		m_StreamPos.nHeadPos = 0;  //
	}
	else
	{
		SourceSeekData(0, SEEK_SET);
		bDone = false;
		for (i = 0; i < 4 && !bDone; i++)
		{
			nFrameSize[i] = NextFrame(true);
			if (nFrameSize[i] == 0)
				break;
			nFramePos[i] = SourceGetPosition() - HEADER_SIZE;

			for (j = 0; j < i; j++)
				if (nFramePos[j] + nFrameSize[j] == nFramePos[i])
				{
					m_StreamPos.nHeadPos = nFramePos[j];
					m_nFrameSize = nFrameSize[j];
					bDone = true;
					break;
				}
		}
		if (!bDone)
			return;

		m_nOutputNum = 1;
		m_AACInfo.nHeaderType = AAC_HEADER_ADTS;
	}

	SeekTime(0);
}

CAACSource::~CAACSource()
{
}

MediaType CAACSource::GetSourceType(void)
{
	return MEDIA_TYPE_AAC;
}

bool CAACSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(AACInfo))
		return false;

	m_AACInfo.nSize = sizeof(AACInfo);
	memcpy(pInfo, &m_AACInfo, m_AACInfo.nSize);
	return true;
}

MediaType CAACSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_AAC;
}

bool CAACSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CAACSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CAACSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;
	
	nSize = NextFrame(false);
	if (nSize == 0)
		return 0;

	if (pData != NULL)
	{
		memcpy(pData, m_pHeader, HEADER_SIZE);
		SourceReadData((char *)pData + HEADER_SIZE, nSize - HEADER_SIZE);
	}
	else
		SourceSeekData(nSize - HEADER_SIZE, SEEK_CUR);
	return nSize;
}

bool CAACSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CAACSource::GetDuration(void)
{
	int nFrameNum;

	if (m_nOutputNum == 0 || m_AACInfo.nHeaderType != AAC_HEADER_ADTS)
		return 0;

	nFrameNum = (SourceGetSize() - m_StreamPos.nHeadPos) / m_nFrameSize;
	return (int)((int64_t)nFrameNum * 1024 * 1000 / m_AACInfo.nSampleRate);
}

int CAACSource::SeekTime(int nTime)
{
	if (m_nOutputNum > 0)
		SourceSeekData(m_StreamPos.nHeadPos, SEEK_SET);
	return 0;
}

int CAACSource::NextFrame(bool bSearch)
{
	static AACProfile profileTable[] = {AAC_PROFILE_MAIN, AAC_PROFILE_LC, AAC_PROFILE_SSR, AAC_PROFILE_LTP};
	int i, nSize;

	if (SourceReadData(m_pHeader, 1) < 1)
		return 0;

	for (i = bSearch? 0x4000 : 1; i != 0; i--)  //0x4000 is max frame size
	{
		if (SourceReadData(m_pHeader + 1, 1) < 1)
			return 0;
		if (m_pHeader[0] == 0xFF && (m_pHeader[1] & 0xF6) == 0xF0)  //sync word
			break;
		m_pHeader[0] = m_pHeader[1];
	}
	if (i == 0)
		return 0;

	SourceReadData(m_pHeader + 2, HEADER_SIZE - 2);
	m_AACInfo.nProfile = profileTable[m_pHeader[2] >> 6];
	m_AACInfo.nSampleRate = sampleRateTable[(m_pHeader[2] >> 2) & 0x0F];  //sample rate
	m_AACInfo.nChannel = ((m_pHeader[2] & 0x01) << 2) | (m_pHeader[3] >> 6);  //channel
	nSize = ((m_pHeader[3] & 0x03) << 11) | (m_pHeader[4] << 3) | (m_pHeader[5] >> 5);  //frame length
	return nSize;
}
