#include "AMRSource.h"
#include <string.h>

CAMRSource::CAMRSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char pBuffer[5];

	SourceReadData(pBuffer, 5);
	if (strncmp(pBuffer, "#!AMR", 5) != 0)
		return;

	m_nOutputNum = 1;
	SeekTime(0);
}

CAMRSource::~CAMRSource()
{
}

MediaType CAMRSource::GetSourceType(void)
{
	return MEDIA_TYPE_AMR;
}

bool CAMRSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType CAMRSource::GetOutputType(int nIndex)
{
	return MEDIA_TYPE_UNKNOWN;
}

bool CAMRSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

size_t CAMRSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CAMRSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CAMRSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CAMRSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CAMRSource::SeekTime(int nTime)
{
	SourceSeekData(0, SEEK_SET);
	return 0;
}
