#include "RealAudioSource.h"
#include <string.h>

CRealAudioSource::CRealAudioSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[5];
	int nVersion;

	memset(&m_RealMediaInfo, 0, sizeof(RealMediaInfo));
	str[4] = '\0';
	SourceReadData(str, 4);
	if (strcmp(str, ".ra\xfd") != 0)
		return;

	nVersion = SourceReadBigEndian(2);
	if (nVersion == 3)
		SourceSeekData(16, SEEK_CUR);
	else if (nVersion == 4)
	{
		SourceSeekData(42, SEEK_CUR);
		m_AudioInfo.nSampleRate = SourceReadBigEndian(2);
		SourceSeekData(4, SEEK_CUR);
		m_AudioInfo.nChannel = SourceReadBigEndian(2);
		SourceSeekData(5, SEEK_CUR);  //Interleaver ID string
		SourceSeekData(5, SEEK_CUR);  //FourCC string
		SourceSeekData(3, SEEK_CUR);  //unknown
	}
	else
		return;

	m_RealMediaInfo.pTitle = ReadString(1);  //title
	m_RealMediaInfo.pAuthor = ReadString(1);  //author
	m_RealMediaInfo.pCopyright = ReadString(1);  //copyright
	m_RealMediaInfo.pComment = ReadString(1);  //comment
	m_nOutputNum = 1;
}

CRealAudioSource::~CRealAudioSource()
{
	delete[] m_RealMediaInfo.pTitle;
	delete[] m_RealMediaInfo.pAuthor;
	delete[] m_RealMediaInfo.pCopyright;
	delete[] m_RealMediaInfo.pComment;
}

MediaType CRealAudioSource::GetSourceType(void)
{
	return MEDIA_TYPE_REALAUDIO;
}

bool CRealAudioSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(RealMediaInfo))
		return false;

	m_RealMediaInfo.nSize = sizeof(RealMediaInfo);
	memcpy(pInfo, &m_RealMediaInfo, m_RealMediaInfo.nSize);
	return true;
}

MediaType CRealAudioSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_UNKNOWN_AUDIO;
}

bool CRealAudioSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

size_t CRealAudioSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CRealAudioSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CRealAudioSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CRealAudioSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CRealAudioSource::SeekTime(int nTime)
{
	return 0;  //not implemented
}

char *CRealAudioSource::ReadString(int nSize)
{
	char *str;

	nSize = SourceReadBigEndian(nSize);
	if (nSize == 0)
		return NULL;

	str = new char[nSize + 1];
	SourceReadData(str, nSize);
	str[nSize] = 0;
	return str;
}
