#include "ProgramAssociationSource.h"
#include <string.h>

CProgramAssociationSource::CProgramAssociationSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i, nSectionLen, nValue;

	memset(&m_Info, 0, sizeof(ProgramAssociationInfo));
	if (SourceReadData() != 0)  //table id should be 0
		return;

	nSectionLen = SourceReadBigEndian(2) & 0x0FFF;  //section length
	SourceSeekData(2, SEEK_CUR);  //transport stream id
	nValue = SourceReadData();
	m_Info.nVersion = (nValue >> 1) & 0x1F;  //version number
	SourceSeekData(2, SEEK_CUR);  //section number
	m_Info.nProgramNum = (nSectionLen - 9) >> 2;  //section length / 4
	m_Info.pInfo = new ProgramInfo[m_Info.nProgramNum];
	for (i = 0; i < m_Info.nProgramNum; i++)
	{
		m_Info.pInfo[i].nProgramNumber = SourceReadBigEndian(2);  //program number
		m_Info.pInfo[i].nPID = SourceReadBigEndian(2) & 0x1FFF;  //network PID or program map PID
	}
}

CProgramAssociationSource::~CProgramAssociationSource()
{
	delete[] m_Info.pInfo;
}

MediaType CProgramAssociationSource::GetSourceType(void)
{
	return MEDIA_TYPE_PROGRAMASSOCIATION;
}

bool CProgramAssociationSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (pInfo->nSize < sizeof(ProgramAssociationInfo))
		return false;

	m_Info.nSize = sizeof(ProgramAssociationInfo);
	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

MediaType CProgramAssociationSource::GetOutputType(int nIndex)
{
	return MEDIA_TYPE_UNKNOWN;
}

bool CProgramAssociationSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

size_t CProgramAssociationSource::GetPosition(int nIndex)
{
	return 0;
}

int CProgramAssociationSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;
}

bool CProgramAssociationSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;
}

int CProgramAssociationSource::GetDuration(void)
{
	return 0;
}

int CProgramAssociationSource::SeekTime(int nTime)
{
	return 0;
}
