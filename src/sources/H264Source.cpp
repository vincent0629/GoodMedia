#include "H264Source.h"
#include <string.h>

CH264Source::CH264Source(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i, nCode;
	int nValue;

	for (i = 0; i < 30; i++)
	{
		nCode = NextSync();
		if (nCode == -1)
			return;

		if ((nCode & 0x9F) == 0x07)
		{
			nValue = SourceReadData();
			if (nValue != 0x42 && nValue != 0x4D && nValue != 0x58 && nValue != 0x64 && nValue != 0x6E && nValue != 0x7A && nValue != 0x90)
				continue;
			if ((SourceReadData() & 0x0F) != 0)
				continue;

			m_H264Info.nWidth = 0;
			m_H264Info.nHeight = 0;
			m_nOutputNum = 1;
			break;
		}
	}

	if (m_nOutputNum > 0)
		SeekTime(0);
}

CH264Source::~CH264Source()
{
}

MediaType CH264Source::GetSourceType(void)
{
	return MEDIA_TYPE_H264;
}

bool CH264Source::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(VideoInfo))
		return false;

	m_H264Info.nSize = sizeof(VideoInfo);
	memcpy(pInfo, &m_H264Info, m_H264Info.nSize);
	return true;
}

MediaType CH264Source::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_H264;
}

bool CH264Source::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CH264Source::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CH264Source::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CH264Source::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //not implemented
}

int CH264Source::GetDuration(void)
{
	return 0;  //not implemented
}

int CH264Source::SeekTime(int nTime)
{
	SourceSeekData(0, SEEK_SET);
	return 0;
}

int CH264Source::NextSync(void)
{
	int nBuffer;
	int i, nCode;

	nBuffer = SourceReadBigEndian(2);
	i = 3000;
	do
	{
		if (--i <= 0)
			return -1;
		if ((nCode = SourceReadData()) == -1)
			return -1;

		nBuffer = ((nBuffer & 0xFFFF) << 8) | nCode;
	} while (nBuffer != 0x000001);
	return SourceReadData();
}
