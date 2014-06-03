#include "AUSource.h"
#include <string.h>
#include <stdint.h>

CAUSource::CAUSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[5];
	int nValue;

	str[4] = '\0';
	SourceReadData(str, 4);
	if (strcmp(str, ".snd") != 0)
		return;

	m_Pos.nHeadPos = SourceReadBigEndian(4);  //data location
	m_Pos.nSize = SourceReadBigEndian(4);  //data size

	nValue = SourceReadBigEndian(4);  //data format
	switch (nValue)
	{
		case 1:
			m_Info.nFormat = PCM_MULAW;
			m_Info.nBitsPerSample = 8;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			m_Info.nFormat = PCM_LINEAR;
			m_Info.nBitsPerSample = 2 << nValue;
			break;
		case 6:
			m_Info.nFormat = PCM_FLOAT;
			m_Info.nBitsPerSample = 32;
			break;
		case 7:
			m_Info.nFormat = PCM_FLOAT;
			m_Info.nBitsPerSample = 64;
			break;
		case 27:
			m_Info.nFormat = PCM_ALAW;
			m_Info.nBitsPerSample = 8;
			break;
		default:
			m_Info.nFormat = PCM_UNKNOWN;
			m_Info.nBitsPerSample = 0;
			break;
	}
	m_Info.nSampleRate = SourceReadBigEndian(4);  //sampling rate
	m_Info.nChannel = SourceReadBigEndian(4);  //channel count
	m_Info.nByteOrder = PCM_LITTLE_ENDIAN;
	m_Info.bSigned = m_Info.nBitsPerSample > 8;

	m_nOutputNum = 1;
	m_nBytesPerSample = m_Info.nBitsPerSample / 8 * m_Info.nChannel;
	m_nDuration = m_nBytesPerSample == 0? 0 : m_Pos.nSize / m_nBytesPerSample * 1000 / m_Info.nSampleRate;
	SeekTime(0);
}

CAUSource::~CAUSource()
{
}

MediaType CAUSource::GetSourceType(void)
{
	return MEDIA_TYPE_AU;
}

bool CAUSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType CAUSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_PCM;
}

bool CAUSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	if (nIndex >= m_nOutputNum || pInfo->nSize < sizeof(PCMInfo))
		return false;

	m_Info.nSize = sizeof(PCMInfo);
	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

size_t CAUSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_Pos.nRelPos;
}

int CAUSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	if (nSize > m_Pos.nSize - m_Pos.nRelPos)
		nSize = m_Pos.nSize - m_Pos.nRelPos;
	nSize = SourceReadData(pData, nSize);
	m_Pos.nRelPos += nSize;
	return nSize;
}

bool CAUSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	bool bResult;

	if (nIndex >= m_nOutputNum)
		return false;

	if (nFrom == SEEK_CUR)
		nOffset += m_Pos.nRelPos;
	else if (nFrom == SEEK_END)
		nOffset += m_Pos.nSize;
	if (nOffset < 0)
	{
		nOffset = 0;
		bResult = false;
	}
	else if (nOffset > m_Pos.nSize)
	{
		nOffset = m_Pos.nSize;
		bResult = false;
	}
	else
		bResult = true;

	m_Pos.nRelPos = nOffset;
	SourceSeekData(m_Pos.nHeadPos + m_Pos.nRelPos, SEEK_SET);
	return bResult;
}

int CAUSource::GetOutputTime(int nIndex)
{
	return nIndex >= m_nOutputNum || m_nBytesPerSample == 0? 0 : m_Pos.nRelPos / m_nBytesPerSample * 1000 / m_Info.nSampleRate;
}

int CAUSource::GetDuration(void)
{
	return m_nOutputNum == 0? 0 : m_nDuration;
}

int CAUSource::SeekTime(int nTime)
{
	if (m_nOutputNum == 0)
		return 0;

	if (nTime > m_nDuration)
		nTime = m_nDuration;
	m_Pos.nRelPos = (int64_t)nTime * m_Info.nSampleRate / 1000 * m_nBytesPerSample;
	SourceSeekData(m_Pos.nHeadPos + m_Pos.nRelPos, SEEK_SET);
	return nTime;
}
