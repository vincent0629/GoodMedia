#include "PCMSource.h"
#include <string.h>
#include <stdint.h>

CPCMSource::CPCMSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	if (m_pSource == NULL)
		return;
	if (m_pSource->GetOutputType(m_nSourceIndex) != MEDIA_TYPE_PCM)
		return;

	m_Info.nSize = sizeof(PCMInfo);
	if (!m_pSource->GetOutputInfo(m_nSourceIndex, &m_Info))
	{
		m_Info.nSampleRate = 8000;
		m_Info.nChannel = 1;
		m_Info.nFormat = PCM_LINEAR;
		m_Info.nBitsPerSample = 16;
		m_Info.nByteOrder = PCM_LITTLE_ENDIAN;
		m_Info.bSigned = true;
	}
	m_nSampleRate = m_Info.nSampleRate;
	m_nBytesPerSample = ((m_Info.nBitsPerSample + 7) >> 3) * m_Info.nChannel;
	m_nOutputNum = 1;
	m_StreamPos.nHeadPos = 0;
	SeekTime(0);
}

CPCMSource::~CPCMSource()
{
}

MediaType CPCMSource::GetSourceType(void)
{
	return MEDIA_TYPE_PCM;
}

bool CPCMSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(PCMInfo))
		return false;

	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

MediaType CPCMSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_PCM;
}

bool CPCMSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CPCMSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CPCMSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	nSize = SourceReadData(pData, nSize);
	m_StreamPos.nRelPos += nSize;
	m_StreamPos.nTime = m_StreamPos.nRelPos / m_nBytesPerSample * 1000 / m_nSampleRate;
	return nSize;
}

bool CPCMSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	bool bResult;

	if (nIndex >= m_nOutputNum)
		return false;

	bResult = SourceSeekData(nOffset, nFrom);
	m_StreamPos.nRelPos = SourceGetPosition();
	m_StreamPos.nTime = m_StreamPos.nRelPos / m_nBytesPerSample * 1000 / m_nSampleRate;
	return bResult;
}

int CPCMSource::GetOutputTime(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nTime;
}

int CPCMSource::GetDuration(void)
{
	return SourceGetSize() / m_nBytesPerSample * 1000 / m_nSampleRate;
}

int CPCMSource::SeekTime(int nTime)
{
	int nPos;

	nPos = (int)((int64_t)nTime * m_nSampleRate / 1000 * m_nBytesPerSample);
	SeekData(0, nPos, SEEK_SET);
	return m_StreamPos.nTime;
}
