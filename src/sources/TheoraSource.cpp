#include "TheoraSource.h"
#include <string.h>

CTheoraSource::CTheoraSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int nValue;
	char str[7];

	nValue = SourceReadData();
	if (nValue != 0x80)
		return;

	str[6] = '\0';
	SourceReadData(str, 6);
	if (strcmp(str, "theora") != 0)
		return;

	m_nOutputNum = 1;
	m_Info.nMajorVersion = SourceReadData();  //major version
	m_Info.nMinorVersion = SourceReadData();  //minor version
	m_Info.nVersionRevision = SourceReadData();  //version revision
	m_Info.nFrameWidth = SourceReadBigEndian(2) * 16;  //width of the frame in macro blocks * 16
	m_Info.nFrameHeight = SourceReadBigEndian(2) * 16;  //height of the frame in macro blocks * 16
	m_Info.nWidth = SourceReadBigEndian(3);  //width of the picture region
	m_Info.nHeight = SourceReadBigEndian(3);  //height of the picture region
	m_Info.nOffsetX = SourceReadData();  //x offset of the picture region
	m_Info.nOffsetY = SourceReadData();  //y offset of the picture region
	nValue = SourceReadBigEndian(4);  //frame rate numerator
	m_Info.fFrameRate = (float)nValue / SourceReadBigEndian(4);  //frame rate numerator / frame rate denominator
}

CTheoraSource::~CTheoraSource()
{
}

MediaType CTheoraSource::GetSourceType(void)
{
	return MEDIA_TYPE_THEORA;
}

bool CTheoraSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(TheoraInfo))
		return false;

	m_Info.nSize = sizeof(TheoraInfo);
	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

MediaType CTheoraSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_THEORA;
}

bool CTheoraSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CTheoraSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CTheoraSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CTheoraSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CTheoraSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CTheoraSource::SeekTime(int nTime)
{
	return 0;  //not implemented
}
