#include "RMIDSource.h"
#include <string.h>

CRMIDSource::CRMIDSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[5];
	int nSize;

	memset(&m_StreamPos, 0, sizeof(m_StreamPos));
	str[4] = '\0';
	SourceReadData(str, 4);
	if (strcmp(str, "RIFF") != 0)
		return;
	SourceSeekData(4, SEEK_CUR);
	SourceReadData(str, 4);
	if (strcmp(str, "RMID") != 0)
		return;

	for (;;)
	{
		if (SourceReadData(str, 4) < 4)
			break;
		nSize = SourceReadLittleEndian(4);
		if (strcmp(str, "data") == 0)
		{
			m_StreamPos.nHeadPos = SourceGetPosition();
			m_StreamPos.nSize = nSize;
			break;
		}
		SourceSeekData(nSize, SEEK_CUR);
	}
	m_nOutputNum = 1;
	SeekTime(0);
}

CRMIDSource::~CRMIDSource()
{
}

MediaType CRMIDSource::GetSourceType(void)
{
	return MEDIA_TYPE_RMID;
}

bool CRMIDSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType CRMIDSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_MIDI;
}

int CRMIDSource::GetDuration(void)
{
	CMediaSource *pSource;

	pSource = GetOutputSource(0);
	return pSource == NULL? 0 : pSource->GetDuration();
}

int CRMIDSource::SeekTime(int nTime)
{
	CMediaSource *pSource;

	pSource = GetOutputSource(0);
	return pSource == NULL? 0 : pSource->SeekTime(nTime);
}

size_t CRMIDSource::GetSize(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nSize;
}

size_t CRMIDSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CRMIDSource::ReadData(int nIndex, void *pData, int nSize)
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

bool CRMIDSource::SeekData(int nIndex, size_t nOffset, int nFrom)
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
	return SourceSeekData(m_StreamPos.nAbsPos, SEEK_SET);
}
