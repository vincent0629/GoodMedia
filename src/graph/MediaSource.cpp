#include "MediaSource.h"
#include "MediaSourceFactory.h"
#include <string.h>
#include <assert.h>

CMediaSource::CMediaSource()
{
	m_pSource = NULL;
	m_nSourceIndex = 0;
	m_nOutputNum = 0;
	m_ppOutputSources = NULL;
	m_pGotOutputSources = NULL;
}

CMediaSource::CMediaSource(CMediaSource *pSource, int nIndex)
{
	m_pSource = pSource;
	m_nSourceIndex = nIndex;
	m_nOutputNum = 0;
	m_ppOutputSources = NULL;
	m_pGotOutputSources = NULL;
	SourceSeekData(0, SEEK_SET);
}

CMediaSource::~CMediaSource()
{
	int i;

	if (m_ppOutputSources != NULL)
	{
		for (i = 0; i < m_nOutputNum; i++)
			delete m_ppOutputSources[i];
		delete[] m_ppOutputSources;
		delete[] m_pGotOutputSources;
	}
}

int CMediaSource::GetOutputNum(void)
{
	return m_nOutputNum;
}

bool CMediaSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

CMediaSource *CMediaSource::GetOutputSource(int nIndex)
{
	MediaType type;

	if (nIndex >= m_nOutputNum)
		return NULL;

	type = GetOutputType(nIndex);
	if (type == GetSourceType())
		return NULL;

	if (m_ppOutputSources == NULL)
	{
		m_ppOutputSources = new CMediaSource *[m_nOutputNum];
		memset(m_ppOutputSources, 0, m_nOutputNum * sizeof(CMediaSource *));
		m_pGotOutputSources = new bool[m_nOutputNum];
		memset(m_pGotOutputSources, 0, m_nOutputNum * sizeof(bool));
	}

	if (m_ppOutputSources[nIndex] == NULL && !m_pGotOutputSources[nIndex])
	{
		m_ppOutputSources[nIndex] = CMediaSourceFactory::Create(type, this, nIndex);
		m_pGotOutputSources[nIndex] = true;
	}
	return m_ppOutputSources[nIndex];
}

int CMediaSource::GetOutputTime(int nIndex)
{
	return 0;
}

size_t CMediaSource::GetSize(int nIndex)
{
	return 0;
}

int CMediaSource::ReadData(int nIndex)
{
	unsigned char nResult;

	return ReadData(nIndex, &nResult, 1) == 0? -1 : nResult;
}

int CMediaSource::ReadLittleEndian(int nIndex, int nSize)
{
	int nResult;

	assert(nSize <= sizeof(int));
	nResult = 0;
	ReadData(nIndex, &nResult, nSize);
	return nResult;
}

int CMediaSource::ReadBigEndian(int nIndex, int nSize)
{
	int i, nData, nResult;

	assert(nSize <= sizeof(int));
	nData = 0;
	ReadData(nIndex, &nData, nSize);
	nResult = 0;
	for (i = 0; i < nSize; i++)
	{
		nResult = (nResult << 8) | (nData & 0xFF);
		nData >>= 8;
	}
	return nResult;
}

size_t CMediaSource::SourceGetSize(void)
{
	return m_pSource->GetSize(m_nSourceIndex);
}

size_t CMediaSource::SourceGetPosition(void)
{
	return m_pSource->GetPosition(m_nSourceIndex);
}

int CMediaSource::SourceReadData(void)
{
	return m_pSource->ReadData(m_nSourceIndex);
}

int CMediaSource::SourceReadData(void *pData, int nSize)
{
	int n, nCount;

	nCount = 0;
	while (nCount < nSize)
	{
		n = m_pSource->ReadData(m_nSourceIndex, pData == NULL? NULL : (char *)pData + nCount, nSize - nCount);
		if (n == 0)
			break;
		nCount += n;
	}
	return nCount;
}

int CMediaSource::SourceReadLittleEndian(int nSize)
{
	return m_pSource->ReadLittleEndian(m_nSourceIndex, nSize);
}

int CMediaSource::SourceReadBigEndian(int nSize)
{
	return m_pSource->ReadBigEndian(m_nSourceIndex, nSize);
}

bool CMediaSource::SourceSeekData(size_t nOffset, int nFrom)
{
	return m_pSource->SeekData(m_nSourceIndex, nOffset, nFrom);
}

int CMediaSource::SourceGetOutputTime(int nIndex)
{
	return m_pSource->GetOutputTime(nIndex);
}

MediaType CMediaSource::DetectOutputType(int nMode, int nIndex)
{
	static MediaType typeTable0[] = {  //container
		MEDIA_TYPE_AVI, MEDIA_TYPE_DAT, MEDIA_TYPE_WAV, MEDIA_TYPE_ASF, MEDIA_TYPE_QUICKTIME, MEDIA_TYPE_REALMEDIA, MEDIA_TYPE_REALAUDIO, MEDIA_TYPE_AIFF, MEDIA_TYPE_AMR, MEDIA_TYPE_RMID, MEDIA_TYPE_MIDI, MEDIA_TYPE_OGG, MEDIA_TYPE_MONKEYSAUDIO, MEDIA_TYPE_FLAC, MEDIA_TYPE_AU, MEDIA_TYPE_MATROSKA, MEDIA_TYPE_BITMAP, MEDIA_TYPE_FLASHVIDEO, MEDIA_TYPE_PNG, 
		MEDIA_TYPE_MPEGTRANSPORT, MEDIA_TYPE_MPEGPROGRAM, 
		MEDIA_TYPE_MPEGVIDEO, MEDIA_TYPE_MLP, MEDIA_TYPE_DTS, MEDIA_TYPE_AC3, MEDIA_TYPE_H264, MEDIA_TYPE_AAC, MEDIA_TYPE_MPEGAUDIO
	};
	static MediaType typeTable1[] = {  //non container
		MEDIA_TYPE_MPEGVIDEO, MEDIA_TYPE_MLP, MEDIA_TYPE_DTS, MEDIA_TYPE_AC3, MEDIA_TYPE_VORBIS, MEDIA_TYPE_THEORA, MEDIA_TYPE_H264, MEDIA_TYPE_AAC, MEDIA_TYPE_MPEGAUDIO
	};
	MediaType *pTypes;
	int i, nNum;
	CMediaSource *pOutputSource;

	if (nMode == 0)
	{
		pTypes = typeTable0;
		nNum = sizeof(typeTable0) / sizeof(MediaType);
	}
	else
	{
		pTypes = typeTable1;
		nNum = sizeof(typeTable1) / sizeof(MediaType);
	}

	for (i = 0; i < nNum; i++)
	{
		pOutputSource = CMediaSourceFactory::Create(pTypes[i], this, nIndex);
		if (pOutputSource == NULL)
			continue;

		if (pOutputSource->GetOutputNum() > 0)  //accept
		{
			if (m_ppOutputSources == NULL)
			{
				m_ppOutputSources = new CMediaSource *[m_nOutputNum];
				memset(m_ppOutputSources, 0, m_nOutputNum * sizeof(CMediaSource *));
				m_pGotOutputSources = new bool[m_nOutputNum];
				memset(m_pGotOutputSources, 0, m_nOutputNum * sizeof(bool));
			}
			m_ppOutputSources[nIndex] = pOutputSource;
			m_pGotOutputSources[nIndex] = true;
			return pOutputSource->GetSourceType();
		}
		else
			delete pOutputSource;
	}

	return MEDIA_TYPE_UNKNOWN;
}

MediaType CMediaSource::AudioType(long nFormatTag)
{
	MediaType type;

	type = MEDIA_TYPE_UNKNOWN_AUDIO;
	switch (nFormatTag)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x06:
		case 0x07:
			type = MEDIA_TYPE_PCM;
			break;
		case 0x08:
			type = MEDIA_TYPE_DTS;
			break;
		case 0x55:
			type = MEDIA_TYPE_MPEGAUDIO;
			break;
		case 0x0A:  //WMA voice
		case 0x160:  //WMA standard v1
		case 0x161:  //WMA standard v2
		case 0x162:  //WMA pro
		case 0x163:  //WMA lossless
			type = MEDIA_TYPE_WMA;
			break;
		case 0x2000:
			type = MEDIA_TYPE_AC3;
			break;
	}
	return type;
}

MediaType CMediaSource::VideoType(long nCompression)
{
	char str[5];
	MediaType type;

	if (nCompression <= 2)
		type = MEDIA_TYPE_BITMAP;
	else
	{
		memcpy(str, &nCompression, 4);
		str[4] = '\0';
		type = MEDIA_TYPE_UNKNOWN_VIDEO;
		switch (str[0])
		{
			case 'D':
				if (strcmp(str, "DIB ") == 0)
					type = MEDIA_TYPE_BITMAP;
				else if (strcmp(str, "DVR ") == 0)
					type = MEDIA_TYPE_DVR;
				break;
			case 'M':
				if (strcmp(str, "MP1V") == 0 || strcmp(str, "MP2V") == 0 || strcmp(str, "MPEG") == 0)
					type = MEDIA_TYPE_MPEGVIDEO;
				else if (strcmp(str, "MP42") == 0 || strcmp(str, "MP43") == 0 || strcmp(str, "MPG4") == 0 || strcmp(str, "MP4V") == 0)
					type = MEDIA_TYPE_MPEG4VIDEO;
				else if (strcmp(str, "MJPG") == 0)
					type = MEDIA_TYPE_MJPEG;
				else if (strncmp(str, "MSS", 3) == 0)
					type = MEDIA_TYPE_WMV;
				break;
			case 'R':
				if (strcmp(str, "RGB ") == 0 || strcmp(str, "RLE ") == 0)
					type = MEDIA_TYPE_BITMAP;
				break;
			case 'V':
				if (strcmp(str, "VSSH") == 0)
					type = MEDIA_TYPE_H264;
				break;
			case 'W':
				if (strncmp(str, "WMV", 3) == 0 || strcmp(str, "WMVP") == 0 || strcmp(str, "WVP2") == 0)
					type = MEDIA_TYPE_WMV;
				else if (strcmp(str, "WVC1") == 0)
					type = MEDIA_TYPE_VC1;
				break;
			case 'Y':
				if (strcmp(str, "YUY2") == 0)
					type = MEDIA_TYPE_BITMAP;
				break;
		}
	}
	return type;
}

CMediaSource *CMediaSource::GetParent(void)
{
	return m_pSource;
}

int CMediaSource::GetIndex(void)
{
	return m_nSourceIndex;
}
