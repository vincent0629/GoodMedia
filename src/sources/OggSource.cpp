#include "OggSource.h"
#include <string.h>

COggSource::COggSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i;
	int nSize;
	unsigned long nSerial;

	for (i = 0; i < 30; i++)
	{
		nSize = NextPage(&nSerial);
		if (nSize < 0)
			break;
		if (m_nHeaderType & 2)  //beginning of stream
			m_nSerials[m_nOutputNum++] = nSerial;
		SourceSeekData(nSize, SEEK_CUR);
	}

	if (m_nOutputNum > 0)
	{
		for (i = 0; i < m_nOutputNum; i++)
			m_OutputType[i] = DetectOutputType(1, i);  //Ogg doesn't know the output types
		SeekTime(0);
	}
}

COggSource::~COggSource()
{
}

MediaType COggSource::GetSourceType(void)
{
	return MEDIA_TYPE_OGG;
}

bool COggSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType COggSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : m_OutputType[nIndex];
}

size_t COggSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos[nIndex].nRelPos;
}

int COggSource::ReadData(int nIndex, void *pData, int nSize)
{
	StreamPos *pPos;
	int nPageSize;
	unsigned long nSerial;

	if (nIndex >= m_nOutputNum)
		return 0;

	pPos = m_StreamPos + nIndex;
	if (pPos->nRelPos >= pPos->nSize)
	{
		for (;;)
		{
			nPageSize = NextPage(&nSerial);
			if (nPageSize == -1)
				return 0;
			if (nPageSize > 0 && nSerial == m_nSerials[nIndex])
				break;
		}
		pPos->nSize += nPageSize;
	}

	if (nSize > pPos->nSize - pPos->nRelPos)
		nSize = pPos->nSize - pPos->nRelPos;
	SourceReadData(pData, nSize);
	pPos->nRelPos += nSize;
	return nSize;
}

bool COggSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	if (nIndex >= m_nOutputNum)
		return false;

	if (nOffset == 0 && nFrom == SEEK_SET)
	{
		SeekTime(0);
		return true;
	}
	else if (nFrom == SEEK_CUR)
	{
		ReadData(nIndex, NULL, nOffset);
		return true;
	}
	return false;  //note implemented
}

int COggSource::GetDuration(void)
{
	return 0;  //not implemented
}

int COggSource::SeekTime(int nTime)
{
	SourceSeekData(0, SEEK_SET);
	memset(&m_StreamPos, 0, sizeof(StreamPos));
	return 0;
}

int COggSource::NextPage(unsigned long *pnSerial)
{
	char str[5];
	int i, nValue, nSize;

	if (SourceReadData(str, 4) < 4)
		return -1;
	str[4] = '\0';
	if (strcmp(str, "OggS") != 0)  //capture pattern
		return -1;

	SourceReadData();  //version
	m_nHeaderType = SourceReadData();  //header type flag
	SourceSeekData(8, SEEK_CUR);  //absolute granule position
	nValue = SourceReadLittleEndian(4);  //stream serial number
	if (pnSerial)
		*pnSerial = nValue;
	SourceSeekData(4, SEEK_CUR);  //page sequence no
	SourceSeekData(4, SEEK_CUR);  //page checksum
	nValue = SourceReadData();  //page segments
	nSize = 0;
	for (i = 0; i < nValue; i++)  //segment table
		nSize += SourceReadData();  //lacing value
	return nSize;
}
