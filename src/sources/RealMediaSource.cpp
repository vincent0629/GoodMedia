#include "RealMediaSource.h"
#include <string.h>

CRealMediaSource::CRealMediaSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[0x100], mime[0x100];
	int i, nHeaderNum, nSize, nVersion;

	m_nDuration = 0;
	memset(&m_RealMediaInfo, 0, sizeof(m_RealMediaInfo));

	SourceReadData(str, 4);  //object id
	str[4] = '\0';
	if (strcmp(str, ".RMF") != 0)
		return;
	nSize = (int)(SourceReadBigEndian(4) + SourceGetPosition() - 8);  //header size
	nVersion = SourceReadBigEndian(2);  //object version
	if (nVersion <= 1)
	{
		nVersion = SourceReadBigEndian(4);  //file version
		nHeaderNum = SourceReadBigEndian(4);  //num headers
	}
	else
		nHeaderNum = 1;
	SourceSeekData(nSize - SourceGetPosition(), SEEK_CUR);  //skip rest of header

	for (i = 0; i < nHeaderNum; i++)
	{
		SourceReadData(str, 4);  //object id
		str[4] = '\0';
		nSize = (int)(SourceReadBigEndian(4) + SourceGetPosition() - 8);  //size
		nVersion = SourceReadBigEndian(2);  //version
		if (strcmp(str, "PROP") == 0)  //properties header
		{
			if (nVersion == 0)
			{
				SourceSeekData(20, SEEK_CUR);
				m_nDuration = SourceReadBigEndian(4);  //duration
			}
		}
		else if (strcmp(str, "MDPR") == 0)
		{
			if (nVersion == 0)
			{
				SourceSeekData(30, SEEK_CUR);
				ReadString(str, 1);  //stream name
				ReadString(mime, 1);  //mime type
				if (strcmp(str, "Audio Stream") == 0)
				{
					m_pOutputTypes[m_nOutputNum] = MEDIA_TYPE_UNKNOWN_AUDIO;
					if (strcmp(mime, "audio/x-pn-realaudio") == 0 || strcmp(mime, "audio/x-pn-multirate-realaudio") == 0)
						m_pOutputTypes[m_nOutputNum] = MEDIA_TYPE_REALAUDIO;
					else if (strcmp(mime, "audio/X-MP3-draft-00") == 0)
						m_pOutputTypes[m_nOutputNum] = MEDIA_TYPE_MPEGAUDIO;
					else if (strcmp(mime, "audio/x-ralf-mpeg4") == 0)
					{
					}
				}
				else if (strcmp(str, "Video Stream") == 0)
				{
					m_pOutputTypes[m_nOutputNum] = MEDIA_TYPE_UNKNOWN_VIDEO;
					if (strcmp(mime, "video/x-pn-realvideo") == 0)
					{
					}
				}
				else
					m_pOutputTypes[m_nOutputNum] = MEDIA_TYPE_UNKNOWN;
				++m_nOutputNum;
			}
		}
		else if (strcmp(str, "CONT") == 0)
		{
			if (nVersion == 0)
			{
				m_RealMediaInfo.pTitle = ReadString(2);  //title
				m_RealMediaInfo.pAuthor = ReadString(2);  //author
				m_RealMediaInfo.pCopyright = ReadString(2);  //copyright
				m_RealMediaInfo.pComment = ReadString(2);  //comment
			}
		}
		SourceSeekData(nSize - SourceGetPosition(), SEEK_CUR);  //skip rest of header
	}

	if (m_nOutputNum > 0)
		SeekTime(0);
}

CRealMediaSource::~CRealMediaSource()
{
	delete[] m_RealMediaInfo.pTitle;
	delete[] m_RealMediaInfo.pAuthor;
	delete[] m_RealMediaInfo.pCopyright;
	delete[] m_RealMediaInfo.pComment;
}

MediaType CRealMediaSource::GetSourceType(void)
{
	return MEDIA_TYPE_REALMEDIA;
}

bool CRealMediaSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(RealMediaInfo))
		return false;

	m_RealMediaInfo.nSize = sizeof(RealMediaInfo);
	memcpy(pInfo, &m_RealMediaInfo, m_RealMediaInfo.nSize);
	return true;
}

MediaType CRealMediaSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : m_pOutputTypes[nIndex];
}

bool CRealMediaSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

int CRealMediaSource::GetDuration(void)
{
	return m_nDuration;
}

int CRealMediaSource::SeekTime(int nTime)
{
	return 0;  //not implemented
}

size_t CRealMediaSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CRealMediaSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CRealMediaSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

void CRealMediaSource::ReadString(char *str, int nSize)
{
	nSize = SourceReadBigEndian(nSize);
	SourceReadData(str, nSize);
	str[nSize] = 0;
}

char *CRealMediaSource::ReadString(int nSize)
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
