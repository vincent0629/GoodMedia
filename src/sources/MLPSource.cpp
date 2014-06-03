#include "MLPSource.h"
#include <string.h>

CMLPSource::CMLPSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	static int pSampleRateTable[] = {48000, 96000, 192000, 0, 0, 0, 0, 0, 44100, 88200, 176400};
	static int pChannelTable[] = {2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1};
	int i, nValue;
	int nFrameSize;
	unsigned long *pnSync;

	nFrameSize = NextSync(true);
	SourceSeekData(nFrameSize - HEADER_SIZE, SEEK_CUR);
	pnSync = (unsigned long *)(m_pHeader + 4);
	for (i = 200; i > 0; i--)
	{
		nFrameSize = NextSync(false);
		if (*pnSync == 0xBA6F72F8 || *pnSync == 0xBB6F72F8)  //major sync
			break;
		SourceSeekData(nFrameSize - HEADER_SIZE, SEEK_CUR);
	}
	if (i <= 0)
		return;

	m_nOutputNum = 1;
	nValue = m_pHeader[8] >> 4;  //audio_sampling_frequency
	m_MLPInfo.nSampleRate = nValue <= 10? pSampleRateTable[nValue] : 0;
	nValue = ((m_pHeader[10] & 0x1F) << 8) | m_pHeader[11];  //8ch_decoder_channel_assignment
	m_MLPInfo.nChannel = 0;
	for (i = 0; i < 13; i++)
	{
		if ((nValue & 1) != 0)
			m_MLPInfo.nChannel += pChannelTable[i];
		nValue >>= 1;
	}
	m_MLPInfo.nStream = m_pHeader[7] == 0xBA? MLP_STREAM_FBA : MLP_STREAM_FBB;
	SeekTime(0);
}

CMLPSource::~CMLPSource()
{
}

MediaType CMLPSource::GetSourceType(void)
{
	return MEDIA_TYPE_MLP;
}

bool CMLPSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MLPInfo))
		return false;

	m_MLPInfo.nSize = sizeof(MLPInfo);
	memcpy(pInfo, &m_MLPInfo, m_MLPInfo.nSize);
	return true;
}

MediaType CMLPSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_MLP;
}

bool CMLPSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CMLPSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CMLPSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CMLPSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CMLPSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CMLPSource::SeekTime(int nTime)
{
	if (m_nOutputNum > 0)
		SourceSeekData(m_StreamPos.nHeadPos, SEEK_SET);
	return 0;
}

int CMLPSource::NextSync(bool bSearch)
{
	int i;
	unsigned long *pnSync;

	if (SourceReadData(m_pHeader, 8) < 8)
		return 0;

	if (bSearch)
	{
		pnSync = (unsigned long *)(m_pHeader + 4);
		for (i = 0x2000; i != 0; i--)  //0x2000 is about max frame size
		{
			if (*pnSync == 0xBA6F72F8 || *pnSync == 0xBB6F72F8)  //format_sync 0xF8726FBA or 0xF8726FBB
				break;
			memmove(m_pHeader, m_pHeader + 1, 7);
			if (SourceReadData(m_pHeader + 7, 1) < 1)
				return 0;
		}
		if (i == 0)
			return 0;
	}

	SourceReadData(m_pHeader + 8, HEADER_SIZE - 8);
	return (((m_pHeader[0] & 0x0F) << 8) | m_pHeader[1]) * 2;  //access unit length * 2
}
