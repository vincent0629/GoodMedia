#include "FLACSource.h"
#include <string.h>
#include <stdint.h>

CFLACSource::CFLACSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[4];
	int nType;
	long nLength;
	size_t nPos;
	int64_t nSamples;
	unsigned long nValue;

	m_nDuration = 0;
	SourceReadData(str, 4);
	if (strncmp(str, "fLaC", 4) != 0)
		return;

	do
	{
		nType = SourceReadData();  //last-metadata-block flag and block type
		nLength = SourceReadBigEndian(3);  //length
		nPos = SourceGetPosition() + nLength;
		switch (nType & 0x7F)  //block type
		{
			case 0:  //STREAMINFO
				SourceSeekData(4, SEEK_CUR);  //block size
				SourceSeekData(6, SEEK_CUR);  //frame size
				nValue = SourceReadBigEndian(4);
				m_FLACInfo.nSampleRate = nValue >> 12;
				m_FLACInfo.nChannel = ((nValue >> 9) & 7) + 1;
				m_FLACInfo.nBitsPerSample = ((nValue >> 4) & 0x1F) + 1;
				nSamples = nValue & 0x0F;
				nSamples = (nSamples << 32) | SourceReadBigEndian(4);
				m_nDuration = nSamples * 1000 / m_FLACInfo.nSampleRate;
				m_nOutputNum = 1;
				break;
		}
		SourceSeekData(nPos - SourceGetPosition(), SEEK_CUR);
	} while ((nType & 0x80) == 0);
}

CFLACSource::~CFLACSource()
{
}

MediaType CFLACSource::GetSourceType(void)
{
	return MEDIA_TYPE_FLAC;
}

bool CFLACSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(FLACInfo))
		return false;

	m_FLACInfo.nSize = sizeof(FLACInfo);
	memcpy(pInfo, &m_FLACInfo, m_FLACInfo.nSize);
	return true;
}

MediaType CFLACSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_FLAC;
}

bool CFLACSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CFLACSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CFLACSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CFLACSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CFLACSource::GetDuration(void)
{
	return m_nDuration;
}

int CFLACSource::SeekTime(int nTime)
{
	return 0;  //not implemented
}
