#include "MPEGVideoSource.h"
#include <string.h>
#include <math.h>
#include <assert.h>

#define PICTURE_START_CODE 0x00
#define SLICE_START_CODE_MIN 0x01
#define SLICE_START_CODE_MAX 0xAF
#define USER_DATA_START_CODE 0xB2
#define SEQUENCE_HEADER_CODE 0xB3
#define SEQUENCE_ERROR_CODE 0xB4
#define EXTENSION_START_CODE 0xB5
#define SEQUENCE_END_CODE 0xB7
#define GROUP_START_CODE 0xB8

CMPEGVideoSource::CMPEGVideoSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i, nCode, nRatio;
	int nValue;
	float frameRateTable[] = {0.0f, 23.976f, 24.0f, 25.0f, 29.97f, 30.0f, 50.0f, 59.94f, 60.0f, 0.0f};
	float ratioTable[] = {0.0f, 1.0f, 0.6735f, 0.7031f, 0.7615f, 0.8055f, 0.8437f, 0.8935f, 0.9375f, 0.9815f, 1.0255f, 1.0695f, 1.1250f, 1.1575f, 1.2015f, 0.0f};
	float fRatio;

	for (i = 0; i < 30; i++)
	{
		nCode = NextSync();
		if (nCode == -1)
			return;

		if (nCode == SEQUENCE_HEADER_CODE)
		{
			nValue = SourceReadBigEndian(3);
			m_MPEGVideoInfo.nWidth = nValue >> 12;
			m_MPEGVideoInfo.nHeight = nValue & 0xFFF;
			nCode = SourceReadData();
			nRatio = (nCode & 0xF0) >> 4;
			m_MPEGVideoInfo.fFrameRate = frameRateTable[nCode & 0x0F];
			nValue = SourceReadBigEndian(4);
			if (nValue & 0x02)
			{
				SourceSeekData(63, SEEK_CUR);  //intra quantiser matrix
				nValue = SourceReadData();
			}
			if (nValue & 0x01)
				SourceSeekData(64, SEEK_CUR);  //non-intra quantiser matrix
			nCode = NextSync();
			if (nCode == -1)
				return;
			m_MPEGVideoInfo.nVersion = nCode == EXTENSION_START_CODE? 2 : 1;
			if (m_MPEGVideoInfo.nVersion == 1)
			{
				m_MPEGVideoInfo.fPelAspectRatio = ratioTable[nRatio];
				fRatio = ratioTable[nRatio] * m_MPEGVideoInfo.nHeight / m_MPEGVideoInfo.nWidth;
				if (fabs(fRatio - 1.0f / 1.0f) < 0.05f)
					m_MPEGVideoInfo.nDispAspectRatio = 0x00010001;
				else if (fabs(fRatio - 3.0f / 4.0f) < 0.05f)
					m_MPEGVideoInfo.nDispAspectRatio = 0x00040003;
				else if (fabs(fRatio - 9.0f / 16.0f) < 0.05f)
					m_MPEGVideoInfo.nDispAspectRatio = 0x00100009;
				else if (fabs(fRatio - 7.0f / 10.0f) < 0.05f)
					m_MPEGVideoInfo.nDispAspectRatio = 0x000A0007;
				else
					m_MPEGVideoInfo.nDispAspectRatio = 0;  //invalid
			}
			else if (m_MPEGVideoInfo.nVersion == 2)
			{
				switch (nRatio)
				{
					case 2:
						m_MPEGVideoInfo.nDispAspectRatio = 0x00040003;
						m_MPEGVideoInfo.fPelAspectRatio = 3.0f * m_MPEGVideoInfo.nWidth / (4.0f * m_MPEGVideoInfo.nHeight);
						break;
					case 3:
						m_MPEGVideoInfo.nDispAspectRatio = 0x00100009;
						m_MPEGVideoInfo.fPelAspectRatio = 9.0f * m_MPEGVideoInfo.nWidth / (16.0f * m_MPEGVideoInfo.nHeight);
						break;
					default:
						m_MPEGVideoInfo.nDispAspectRatio = (m_MPEGVideoInfo.nWidth << 16) | m_MPEGVideoInfo.nHeight;
						m_MPEGVideoInfo.fPelAspectRatio = 1.0f;
						break;
				}
			}
			m_nOutputNum = 1;
			break;
		}  //if
		else if (nCode <= 0xB8)
			i = 0;
	}  //for

	if (m_nOutputNum > 0)
		SeekTime(0);
}

CMPEGVideoSource::~CMPEGVideoSource()
{
}

MediaType CMPEGVideoSource::GetSourceType(void)
{
	return MEDIA_TYPE_MPEGVIDEO;
}

bool CMPEGVideoSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MPEGVideoInfo))
		return false;

	m_MPEGVideoInfo.nSize = sizeof(MPEGVideoInfo);
	memcpy(pInfo, &m_MPEGVideoInfo, m_MPEGVideoInfo.nSize);
	return true;
}

MediaType CMPEGVideoSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_MPEGVIDEO;
}

bool CMPEGVideoSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CMPEGVideoSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CMPEGVideoSource::ReadData(int nIndex, void *pData, int nSize)
{
	int nCode;
	int nResult;
	unsigned char *pcData;

	if (nIndex >= m_nOutputNum)
		return 0;

	pcData = (unsigned char *)pData;
	if ((m_nBuffer & 0xFFFFFF00) == 0x00000100)
		nCode = m_nBuffer & 0xFF;
	else
	{
		nCode = NextSync();
		if (nCode == -1)
			return 0;
	}
	if (pcData)
	{
		pcData[0] = 0x00;
		pcData[1] = 0x00;
		pcData[2] = 0x01;
		pcData[3] = nCode;
		pcData += 4;
	}
	nResult = 4;
	m_nBuffer = SourceReadBigEndian(4);
	if (nCode != PICTURE_START_CODE)  //begin with non-picture-start-code, end with picture-start-code
	{
		while (m_nBuffer != 0x00000100 | PICTURE_START_CODE)
		{
			if (pcData)
			{
				*pcData = m_nBuffer >> 24;
				++pcData;
			}
			++nResult;
			nCode = SourceReadData();
			if (nCode == -1)
				break;
			m_nBuffer = (m_nBuffer << 8) | nCode;
		}
	}
	else  //begin with picture-start-code, end with non-slice-start-code
	{
		while (m_nBuffer < (0x00000100 | SLICE_START_CODE_MIN) || m_nBuffer > (0x00000100 | SLICE_START_CODE_MAX))
		{
			if (pcData)
			{
				*pcData = m_nBuffer >> 24;
				++pcData;
			}
			++nResult;
			nCode = SourceReadData();
			if (nCode == -1)
				break;
			m_nBuffer = (m_nBuffer << 8) | nCode;
		}
		for (;;)
		{
			if (pcData)
			{
				*pcData = m_nBuffer >> 24;
				++pcData;
			}
			++nResult;
			nCode = SourceReadData();
			if (nCode == -1)
				break;
			m_nBuffer = (m_nBuffer << 8) | nCode;
			if ((m_nBuffer & 0xFFFFFF00) == 0x00000100 && ((m_nBuffer & 0xFF) < SLICE_START_CODE_MIN || (m_nBuffer & 0xFF) > SLICE_START_CODE_MAX))
				break;
		}
	}
	assert(nResult <= nSize);
	return nResult;
}

bool CMPEGVideoSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return nIndex >= m_nOutputNum? false : SourceSeekData(nOffset, nFrom);
}

int CMPEGVideoSource::GetDuration(void)
{
	return 0;  //not implemented
}

int CMPEGVideoSource::SeekTime(int nTime)
{
	SourceSeekData(0, SEEK_CUR);
	m_nBuffer = 0;
	return 0;
}

int CMPEGVideoSource::GetOutputTime(int nIndex)
{
	return SourceGetOutputTime(nIndex);
}

int CMPEGVideoSource::NextSync(void)
{
	int nBuffer;
	int i, nCode;

	nBuffer = SourceReadBigEndian(2);
	i = 4096;
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
