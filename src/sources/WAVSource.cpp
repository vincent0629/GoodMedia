#include "WAVSource.h"
#include "PCMInfo.h"
#include <string.h>

CWAVSource::CWAVSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char pBuffer[4];
	int nSize, nValue;
	bool bFormat;
	int nBytesPerSample;

	m_nDuration = 0;

	SourceReadData(pBuffer, 4);
	if (strncmp(pBuffer, "RIFF", 4) != 0)  //RIFF
		return;
	SourceSeekData(4, SEEK_CUR);  //RIFF size
	SourceReadData(pBuffer, 4);
	if (strncmp(pBuffer, "WAVE", 4) != 0)  //WAVE chunk
		return;

	bFormat = false;
	for (;;)
	{
		if (SourceReadData(pBuffer, 4) < 4)  //chunk id
			return;
		nSize = SourceReadLittleEndian(4);  //chunk size
		if (!bFormat && strncmp(pBuffer, "fmt ", 4) == 0)  //format chunk
		{
			bFormat = true;
			nValue = SourceReadLittleEndian(4);
			m_WAVInfo.nFormatTag = nValue & 0xFFFF;
			m_WAVInfo.nChannel = nValue >> 16;
			m_WAVInfo.nSampleRate = SourceReadLittleEndian(4);
			SourceSeekData(6, SEEK_CUR);
			nValue = SourceReadLittleEndian(2);
			m_WAVInfo.nBitsPerSample = nValue & 0xFF;
			nSize -= 16;  //rest of chunk
		}
		else if (bFormat && strncmp(pBuffer, "data", 4) == 0)  //data chunk
		{
			m_StreamPos.nHeadPos = SourceGetPosition();
			m_StreamPos.nSize = nSize;
			nBytesPerSample = ((m_WAVInfo.nBitsPerSample + 7) >> 3) * m_WAVInfo.nChannel;
			if (nBytesPerSample != 0)
				m_nDuration = nSize / nBytesPerSample * 1000 / m_WAVInfo.nSampleRate;
			break;
		}

		SourceSeekData(nSize, SEEK_CUR);  //skip the chunk
	}

	m_nOutputNum = 1;
	SeekTime(0);
}

CWAVSource::~CWAVSource()
{
}

MediaType CWAVSource::GetSourceType(void)
{
	return MEDIA_TYPE_WAV;
}

bool CWAVSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(WAVInfo))
		return false;

	m_WAVInfo.nSize = sizeof(WAVInfo);
	memcpy(pInfo, &m_WAVInfo, m_WAVInfo.nSize);
	return true;
}

MediaType CWAVSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : AudioType(m_WAVInfo.nFormatTag);
}

bool CWAVSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	MediaType type;
	PCMInfo *pPCMInfo;

	type = GetOutputType(nIndex);
	if (type != MEDIA_TYPE_PCM)
		return false;
	if (pInfo->nSize < sizeof(PCMInfo))
		return false;

	memcpy(pInfo, &m_WAVInfo, sizeof(AudioInfo));
	pPCMInfo = (PCMInfo *)pInfo;
	pPCMInfo->nSize = sizeof(PCMInfo);
	switch (m_WAVInfo.nFormatTag)
	{
		case 0:
		case 1:
			pPCMInfo->nFormat = PCM_LINEAR;
			break;
		case 3:
			pPCMInfo->nFormat = PCM_FLOAT;
			break;
		case 6:
			pPCMInfo->nFormat = PCM_ALAW;
			break;
		case 7:
			pPCMInfo->nFormat = PCM_MULAW;
			break;
		default:
			pPCMInfo->nFormat = PCM_UNKNOWN;
			break;
	}
	pPCMInfo->nBitsPerSample = m_WAVInfo.nBitsPerSample;
	pPCMInfo->nByteOrder = PCM_LITTLE_ENDIAN;
	pPCMInfo->bSigned = m_WAVInfo.nBitsPerSample > 8;
	return true;
}

size_t CWAVSource::GetSize(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nSize;
}

size_t CWAVSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CWAVSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	if (m_StreamPos.nRelPos + nSize > m_StreamPos.nSize)
		nSize = (int)(m_StreamPos.nSize - m_StreamPos.nRelPos);
	nSize = SourceReadData(pData, nSize);
	m_StreamPos.nAbsPos += nSize;
	m_StreamPos.nRelPos += nSize;
	return nSize;
}

bool CWAVSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	bool bResult;

	if (nIndex >= m_nOutputNum)
		return false;

	if (nFrom == SEEK_CUR)
		nOffset += m_StreamPos.nRelPos;
	else if (nFrom == SEEK_END)
		nOffset += m_StreamPos.nSize;
	if (nOffset < 0)
	{
		nOffset = 0;
		bResult = false;
	}
	else if (nOffset > m_StreamPos.nSize)
	{
		nOffset = m_StreamPos.nSize;
		bResult = false;
	}
	else
		bResult = true;

	m_StreamPos.nRelPos = nOffset;
	m_StreamPos.nAbsPos = m_StreamPos.nHeadPos + m_StreamPos.nRelPos;
	SourceSeekData(m_StreamPos.nAbsPos, SEEK_SET);
	return bResult;
}

int CWAVSource::GetOutputTime(int nIndex)
{
	return 0;
}

int CWAVSource::GetDuration(void)
{
	CMediaSource *pSource;

	if (m_nDuration != 0)
		return m_nDuration;

	pSource = GetOutputSource(0);
	return pSource == NULL? 0 : pSource->GetDuration();
}

int CWAVSource::SeekTime(int nTime)
{
	CMediaSource *pSource;

	if (m_ppOutputSources != NULL)
	{
		pSource = GetOutputSource(0);
		if (pSource != NULL)
			return pSource->SeekTime(nTime);
	}

	m_StreamPos.nRelPos = 0;
	m_StreamPos.nAbsPos = m_StreamPos.nHeadPos;
	SourceSeekData(m_StreamPos.nAbsPos, SEEK_SET);
	return 0;
}
