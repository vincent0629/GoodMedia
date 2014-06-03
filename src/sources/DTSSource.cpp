#include "DTSSource.h"
#include <string.h>

CDTSSource::CDTSSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i;
	int nFramePos[3];

	memset(&m_DTSInfo, 0, sizeof(DTSInfo));
	m_nFrameSize = 0;
	for (i = 0; i < 3; i++)
	{
		if (NextSync(true) == 0)
			return;
		nFramePos[i] = SourceGetPosition() - HEADER_SIZE;
	}
	if (nFramePos[1] - nFramePos[0] != nFramePos[2] - nFramePos[1])
		return;

	m_nOutputNum = 1;
	m_StreamPos.nHeadPos = nFramePos[0];
	m_nFrameSize = nFramePos[1] - nFramePos[0];
	SeekTime(0);
}

CDTSSource::~CDTSSource()
{
}

MediaType CDTSSource::GetSourceType(void)
{
	return MEDIA_TYPE_DTS;
}

bool CDTSSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(DTSInfo))
		return false;

	m_DTSInfo.nSize = sizeof(DTSInfo);
	memcpy(pInfo, &m_DTSInfo, m_DTSInfo.nSize);
	return true;
}

MediaType CDTSSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_DTS;
}

bool CDTSSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CDTSSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CDTSSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CDTSSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CDTSSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CDTSSource::SeekTime(int nTime)
{
	SourceSeekData(0, SEEK_SET);
	return 0;
}

int CDTSSource::NextSync(bool bSearch)
{
	static int nChannelTable[] = {1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8};
	static int nSampleRateTable[] = {0, 8000, 16000, 32000, 0, 0, 11025, 22050, 44100, 0, 0, 12000, 24000, 48000, 0, 0};
	static int nBitsPerSampleTable[] = {16, 16, 20, 20, 0, 24, 24, 0};
	int i, nValue;
	unsigned long *pnSync;

	if (SourceReadData(m_pHeader, 3) < 3)
		return 0;

	pnSync = (unsigned long *)m_pHeader;
	for (i = bSearch? 16384 : 1; i != 0; i--)  //16384 is max frame size
	{
		if (SourceReadData(m_pHeader + 3, 1) < 1)
			return 0;
		if (*pnSync == 0x0180FE7F)  //0x7FFE8001
			break;
		memmove(m_pHeader, m_pHeader + 1, 3);
	}
	if (i == 0)
		return 0;

	if (SourceReadData(m_pHeader + 4, HEADER_SIZE - 4) < HEADER_SIZE - 4)
		return 0;
	nValue = ((m_pHeader[7] & 0x0F) << 2) | (m_pHeader[8] >> 6);  //audio channel arrangement
	m_DTSInfo.nChannel = nValue < sizeof(nChannelTable) / sizeof(nChannelTable[0])? nChannelTable[nValue] : 0;
	nValue = (m_pHeader[8] & 0x3C) >> 2;  //core audio sampling frequency
	m_DTSInfo.nSampleRate = nSampleRateTable[nValue];
	nValue = (m_pHeader[10] & 0x10) >> 4;  //extended coding flag
	if (nValue == 1)
	{
		nValue = m_pHeader[10] >> 5;  //extension audio descriptor flag
	}
	if ((m_pHeader[4] & 0x02) != 0)  //CRC present flag
		nValue = ((m_pHeader[13] & 0x01) << 2) | (m_pHeader[14] >> 6);  //source PCM resolution
	else
		nValue = ((m_pHeader[11] & 0x01) << 2) | (m_pHeader[12] >> 6);  //source PCM resolution
	m_DTSInfo.nBitsPerSample = nBitsPerSampleTable[nValue];

	nValue = ((m_pHeader[5] & 0x03) << 12) | (m_pHeader[6] << 4) | (m_pHeader[7] >> 4);  //primary frame byte size, this value is not reliable
	return m_nFrameSize == 0? nValue + 1 : m_nFrameSize;
}
