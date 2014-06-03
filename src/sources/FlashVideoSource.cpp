#include "FlashVideoSource.h"
#include <string.h>

CFlashVideoSource::CFlashVideoSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[4];
	int nAudioNum, nVideoNum;
	int nTagType, nValue;
	size_t nPos;

	str[3] = '\0';
	SourceReadData(str, 3);  //Signature
	if (strcmp(str, "FLV") != 0)
		return;

	m_Info.nVersion = SourceReadData();  //Version
	nValue = SourceReadData();  //TypeFlags
	nAudioNum = (nValue >> 2) & 1;
	if (nAudioNum)
		++m_nOutputNum;
	nVideoNum = nValue & 1;
	if (nVideoNum)
		++m_nOutputNum;
	m_AudioType = MEDIA_TYPE_UNKNOWN;
	m_VideoType = MEDIA_TYPE_UNKNOWN;
	SourceSeekData(SourceReadBigEndian(4), SEEK_SET);  //DataOffset
	m_StreamPos[0].nHeadPos = m_StreamPos[1].nHeadPos = SourceGetPosition();

	while (nAudioNum > 0 || nVideoNum > 0)
	{
		nTagType = NextTag(&nValue);
		nPos = SourceGetPosition() + nValue;
		if (nTagType == 8)  //audio tags
		{
			if (nAudioNum > 0)
			{
				--nAudioNum;
				nValue = SourceReadData();
				m_PCMInfo.nSampleRate = 44100 >> (3 - (nValue >> 2) & 3);
				m_PCMInfo.nBitsPerSample = (nValue & 2)? 16 : 8;
				m_PCMInfo.nChannel = (nValue & 1) + 1;
				m_PCMInfo.bSigned = m_PCMInfo.nBitsPerSample > 8;
				m_PCMInfo.nByteOrder = PCM_LITTLE_ENDIAN;
				nValue = nValue >> 4;  //SoundFormat
				if (nValue == 0)
				{
					m_AudioType = MEDIA_TYPE_PCM;
					m_PCMInfo.nFormat = PCM_LINEAR;
				}
				else if (nValue == 1)
				{
					m_AudioType = MEDIA_TYPE_PCM;
					m_PCMInfo.nFormat = PCM_UNKNOWN;  //ADPCM
				}
				else if (nValue == 2)
					m_AudioType = MEDIA_TYPE_MPEGAUDIO;
				else if (nValue == 3)
				{
					m_AudioType = MEDIA_TYPE_PCM;
					m_PCMInfo.nFormat = PCM_LINEAR;
				}
				else
					m_AudioType = MEDIA_TYPE_UNKNOWN_AUDIO;
			}
		}
		else if (nTagType == 9)  //video tags
		{
			if (nVideoNum > 0)
			{
				--nVideoNum;
				m_VideoType = MEDIA_TYPE_UNKNOWN_VIDEO;
			}
		}
		SourceSeekData(nPos - SourceGetPosition(), SEEK_CUR);  //skip Data
	}

	if (m_nOutputNum > 0)
		SeekTime(0);
}

CFlashVideoSource::~CFlashVideoSource()
{
}

MediaType CFlashVideoSource::GetSourceType(void)
{
	return MEDIA_TYPE_FLASHVIDEO;
}

bool CFlashVideoSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(FlashVideoInfo))
		return false;

	m_Info.nSize = sizeof(FlashVideoInfo);
	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

MediaType CFlashVideoSource::GetOutputType(int nIndex)
{
	if (nIndex == 0)
	{
		if (m_AudioType != MEDIA_TYPE_UNKNOWN)
			return m_AudioType;
		if (m_VideoType != MEDIA_TYPE_UNKNOWN)
			return m_VideoType;
	}
	else if (nIndex == 1)
	{
		if (m_AudioType != MEDIA_TYPE_UNKNOWN && m_VideoType != MEDIA_TYPE_UNKNOWN)
			return m_VideoType;
	}
	return MEDIA_TYPE_UNKNOWN;
}

bool CFlashVideoSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	if (nIndex >= m_nOutputNum)
		return false;

	if (nIndex == 0 && m_AudioType == MEDIA_TYPE_PCM)
		if (pInfo->nSize >= sizeof(PCMInfo))
		{
			m_PCMInfo.nSize = sizeof(PCMInfo);
			memcpy(pInfo, &m_PCMInfo, m_PCMInfo.nSize);
			return true;
		}
	return false;
}

size_t CFlashVideoSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos[nIndex].nRelPos;
}

int CFlashVideoSource::ReadData(int nIndex, void *pData, int nSize)
{
	StreamPos *pPos;
	int nTargetType, nTagType;
	int nLength;

	if (nIndex >= m_nOutputNum)
		return 0;

	pPos = m_StreamPos + nIndex;
	SourceSeekData(pPos->nAbsPos, SEEK_SET);

	nTargetType = nIndex == 0 && m_AudioType != MEDIA_TYPE_UNKNOWN? 8 : 9;  //audio or video
	if (pPos->nRelPos >= pPos->nSize)
		for (;;)
		{
			nTagType = NextTag(&nLength);
			if (nTagType == -1)
				break;
			if (nTagType == nTargetType)
			{
				SourceSeekData(1, SEEK_CUR);
				--nLength;
				pPos->nSize += nLength;
				break;
			}
			SourceSeekData(nLength, SEEK_CUR);
		}

	if (pPos->nRelPos < pPos->nSize)
	{
		if (nSize > pPos->nSize - pPos->nRelPos)
			nSize = (int)(pPos->nSize - pPos->nRelPos);
		if (pData)
			nSize = SourceReadData(pData, nSize);
		else
			SourceSeekData(nSize, SEEK_CUR);
		pPos->nRelPos += nSize;
	}
	else
		nSize = 0;
	pPos->nAbsPos = SourceGetPosition();

	return nSize;
}

bool CFlashVideoSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	int nSize;

	if (nIndex >= m_nOutputNum)
		return false;

	if (nOffset > 0 && nFrom == SEEK_CUR)
	{
		while (nOffset > 0)
		{
			nSize = ReadData(nIndex, NULL, nOffset);
			if (nSize == 0)
				break;
			nOffset -= nSize;
		}
		return nOffset == 0;
	}
	return false;
}

int CFlashVideoSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CFlashVideoSource::SeekTime(int nTime)
{
	int i;

	for (i = 0; i < 2; i++)
	{
		m_StreamPos[i].nAbsPos = m_StreamPos[i].nHeadPos;
		m_StreamPos[i].nRelPos = 0;
		m_StreamPos[i].nSize = 0;
	}
	return 0;
}

int CFlashVideoSource::NextTag(int *pnLength)
{
	int nTagType;

	SourceSeekData(4, SEEK_CUR);  //PreviousTagSize
	nTagType = SourceReadData();  //TagType
	if (nTagType == -1)
		return -1;

	if (pnLength)
		*pnLength = SourceReadBigEndian(3);  //DataSize
	else
		SourceSeekData(3, SEEK_CUR);
	SourceSeekData(3, SEEK_CUR);  //Timestamp
	SourceSeekData(1, SEEK_CUR);  //TimeStampExtended
	SourceSeekData(3, SEEK_CUR);  //StreamID
	return nTagType;
}
