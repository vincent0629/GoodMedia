#include "MonkeysAudioSource.h"
#include <string.h>

CMonkeysAudioSource::CMonkeysAudioSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char pBuffer[5];

	pBuffer[4] = '\0';
	SourceReadData(pBuffer, 4);
	if (strcmp(pBuffer, "MAC ") != 0)
		return;

	m_nOutputNum = 1;
}

CMonkeysAudioSource::~CMonkeysAudioSource()
{
}

MediaType CMonkeysAudioSource::GetSourceType(void)
{
	return MEDIA_TYPE_MONKEYSAUDIO;
}

bool CMonkeysAudioSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType CMonkeysAudioSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_MONKEYSAUDIO;
}

bool CMonkeysAudioSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

size_t CMonkeysAudioSource::GetPosition(int nIndex)
{
	return 0;
}

int CMonkeysAudioSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;
}

bool CMonkeysAudioSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;
}

int CMonkeysAudioSource::GetDuration(void)
{
	return 0;
}

int CMonkeysAudioSource::SeekTime(int nTime)
{
	return 0;
}
