#include "ProgramMapSource.h"
#include <string.h>

CProgramMapSource::CProgramMapSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int nSectionLen, nInfoLen;

	memset(&m_Info, 0, sizeof(m_Info));
	if (SourceReadData() != 2)  //table id should be 2
		return;

	nSectionLen = SourceReadBigEndian(2) & 0x0FFF;  //section length
	m_Info.nProgramNumber = SourceReadBigEndian(2);  //program number
	m_Info.nVersion = (SourceReadData() >> 1) & 0x1F;  //version number
	SourceSeekData(4, SEEK_CUR);
	nInfoLen = SourceReadBigEndian(2) & 0xFFF;  //program info length
	SourceSeekData(nInfoLen, SEEK_CUR);  //descriptor
	nSectionLen -= 13 + nInfoLen;
	m_Info.nStreamNum = 0;
	m_Info.pInfo = new StreamInfo[nSectionLen / 5];
	while (nSectionLen > 0)
	{
		m_Info.pInfo[m_Info.nStreamNum].nType = SourceReadData();  //stream type
		m_Info.pInfo[m_Info.nStreamNum].nPID = SourceReadBigEndian(2) & 0x1FFF;  //elementary PID
		nInfoLen = SourceReadBigEndian(2) & 0x0FFF;
		SourceSeekData(nInfoLen, SEEK_CUR);
		++m_Info.nStreamNum;
		nSectionLen -= 5 + nInfoLen;
	}
}

CProgramMapSource::~CProgramMapSource()
{
	delete[] m_Info.pInfo;
}

MediaType CProgramMapSource::GetSourceType(void)
{
	return MEDIA_TYPE_PROGRAMMAP;
}

bool CProgramMapSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (pInfo->nSize < sizeof(ProgramMapInfo))
		return false;

	m_Info.nSize = sizeof(ProgramMapInfo);
	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

MediaType CProgramMapSource::GetOutputType(int nIndex)
{
	return MEDIA_TYPE_UNKNOWN;
}

bool CProgramMapSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

size_t CProgramMapSource::GetPosition(int nIndex)
{
	return 0;
}

int CProgramMapSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;
}

bool CProgramMapSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;
}

int CProgramMapSource::GetDuration(void)
{
	return 0;
}

int CProgramMapSource::SeekTime(int nTime)
{
	return 0;
}
