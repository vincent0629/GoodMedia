#include "XXXSource.h"
#include <string.h>

CXXXSource::CXXXSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
}

CXXXSource::~CXXXSource()
{
}

MediaType CXXXSource::GetSourceType(void)
{
	return MEDIA_TYPE_XXX;
}

bool CXXXSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType CXXXSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_UNKNOWN_AUDIO;
}

bool CXXXSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	if (nIndex >= m_nOutputNum || pInfo->nSize < sizeof(AudioInfo))
		return false;

	m_AudioInfo.nSize = sizeof(AudioInfo);
	memcpy(pInfo, &m_AudioInfo, m_AudioInfo.nSize);
	return true;
}

size_t CXXXSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CXXXSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CXXXSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CXXXSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CXXXSource::SeekTime(int nTime)
{
	return 0;  //not implemented
}
