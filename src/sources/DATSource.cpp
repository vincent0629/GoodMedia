#include "DATSource.h"
#include <string.h>

#define SECTOR_SIZE 2352
#define PACK_SIZE 2324

CDATSource::CDATSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char pBuffer[5];
	int nSector;

	m_nDuration = 0;

	pBuffer[4] = '\0';
	SourceReadData(pBuffer, 4);
	if (strcmp(pBuffer, "RIFF") != 0)
		return;
	SourceSeekData(4, SEEK_CUR);
	SourceReadData(pBuffer, 4);
	if (strcmp(pBuffer, "CDXA") != 0)
		return;

	m_nOutputNum = 1;
	SourceSeekData(24, SEEK_CUR);  //format
	SourceReadData(pBuffer, 4);  //should be data
	SourceSeekData(30 * SECTOR_SIZE, SEEK_CUR);  //skip empty sector
	m_StreamPos.nHeadPos = SourceGetPosition();
	nSector = (int)(SourceGetSize() - m_StreamPos.nHeadPos) / SECTOR_SIZE;
	m_nDuration = nSector * 1000 / 75;
	m_nSize = nSector * PACK_SIZE;

	SeekTime(0);
}

CDATSource::~CDATSource()
{
}

MediaType CDATSource::GetSourceType(void)
{
	return MEDIA_TYPE_DAT;
}

bool CDATSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType CDATSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_MPEGPROGRAM;
}

size_t CDATSource::GetSize(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_nSize;
}

size_t CDATSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CDATSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	if (m_StreamPos.nRelPos >= m_StreamPos.nSize)
	{
		if (SourceSeekData(SECTOR_SIZE - PACK_SIZE, SEEK_CUR))
			m_StreamPos.nSize += PACK_SIZE;
	}
	if (m_StreamPos.nRelPos < m_StreamPos.nSize)
	{
		if (nSize > m_StreamPos.nSize - m_StreamPos.nRelPos)
			nSize = (int)(m_StreamPos.nSize - m_StreamPos.nRelPos);
		nSize = SourceReadData(pData, nSize);
		m_StreamPos.nRelPos += nSize;
		return nSize;
	}
	return 0;
}

bool CDATSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	size_t n;

	if (nIndex >= m_nOutputNum)
		return false;

	if (nFrom == SEEK_CUR)
	{
		nFrom = SEEK_SET;
		nOffset += m_StreamPos.nRelPos;
	}
	if (nFrom == SEEK_SET)
	{
		if (nOffset < 0 || nOffset > m_nSize)
			return false;
	}
	else
		return false;

	if (nOffset == 0)
		SeekTime(0);
	else if (nOffset > m_StreamPos.nRelPos)
	{
		nOffset -= m_StreamPos.nRelPos;
		if (nOffset >= m_StreamPos.nSize - m_StreamPos.nRelPos)
			nOffset -= ReadData(nIndex, NULL, m_StreamPos.nSize - m_StreamPos.nRelPos);
		while (nOffset >= PACK_SIZE)
			nOffset -= ReadData(nIndex, NULL, PACK_SIZE);
		if (nOffset > 0)
			ReadData(nIndex, NULL, nOffset);
	}
	else if (nOffset < m_StreamPos.nRelPos)
	{
		n = SourceGetPosition();
		n = nOffset / PACK_SIZE;
		m_StreamPos.nRelPos = nOffset;
		m_StreamPos.nSize = n * PACK_SIZE;
		SourceSeekData(m_StreamPos.nHeadPos + n * SECTOR_SIZE + nOffset - n * PACK_SIZE, SEEK_SET);
	}
	return true;
}

int CDATSource::GetDuration(void)
{
	return m_nDuration;
}

int CDATSource::SeekTime(int nTime)
{
	if (m_nOutputNum > 0)
	{
		SourceSeekData(m_StreamPos.nHeadPos, SEEK_SET);
		m_StreamPos.nRelPos = m_StreamPos.nSize = 0;
	}
	return 0;
}
