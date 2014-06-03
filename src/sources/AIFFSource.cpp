#include "AIFFSource.h"
#include "PCMInfo.h"
#include <string.h>

CAIFFSource::CAIFFSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[5], *pCompression;
	int nSize, nNum, nValue;
	size_t nPos;
	int nExp;
	float fValue, f2;

	m_nDuration = 0;
	m_OutputType = MEDIA_TYPE_UNKNOWN_AUDIO;
	memset(&m_AIFFInfo, 0, sizeof(AIFFInfo));
	str[4] = '\0';
	SourceReadData(str, 4);
	if (strcmp(str, "FORM") != 0)
		return;
	SourceSeekData(4, SEEK_CUR);  //size
	SourceReadData(str, 4);
	if (strcmp(str, "AIFF") == 0)
		m_bCompressed = false;
	else if (strcmp(str, "AIFC") == 0)
		m_bCompressed = true;
	else
		return;

	for (;;)
	{
		if (SourceReadData(str, 4) < 4)  //chunk ID
			break;
		while (str[0] == '\0')
		{
			str[0] = str[1];
			str[1] = str[2];
			str[2] = str[3];
			str[3] = SourceReadData();
		}
		nSize = SourceReadBigEndian(4);  //chunk size
		nPos = SourceGetPosition() + nSize;
		if (strcmp(str, "COMM") == 0)
		{
			m_PCMInfo.nChannel = SourceReadBigEndian(2);
			nNum = SourceReadBigEndian(4);  //number of sample frames
			m_PCMInfo.nBitsPerSample = SourceReadBigEndian(2);
			//80 bit IEEE Standard 754 floating point number
			nExp = SourceReadBigEndian(2) - 16383;
			nValue = SourceReadBigEndian(4);
			SourceSeekData(4, SEEK_CUR);
			fValue = 0.0f;
			f2 = 1.0f;
			while (nValue != 0)
			{
				if (nValue & 0x80000000)
					fValue += f2;
				nValue <<= 1;
				f2 /= 2.0f;
			}
			m_PCMInfo.nSampleRate = fValue * (1 << nExp) + 0.5f;
			m_nDuration = nNum * 1000 / m_PCMInfo.nSampleRate;
			if (m_bCompressed)
			{
				SourceReadData(m_AIFFInfo.pCompressionType, 4);  //compression type
				m_AIFFInfo.pCompressionType[4] = '\0';
				nValue = SourceReadData();  //length of compression type name
				if (nValue > 0)
				{
					m_AIFFInfo.pCompressionName = new char[nValue + 1];
					SourceReadData(m_AIFFInfo.pCompressionName, nValue);  //compression type name
					m_AIFFInfo.pCompressionName[nValue] = '\0';
				}

				pCompression = m_AIFFInfo.pCompressionType;
				if (strcmp(pCompression, "NONE") == 0)
				{
					m_OutputType = MEDIA_TYPE_PCM;
					m_PCMInfo.nFormat = PCM_LINEAR;
				}
				else if (strcmp(pCompression, "fl32") == 0)
				{
					m_OutputType = MEDIA_TYPE_PCM;
					m_PCMInfo.nFormat = PCM_FLOAT;
				}
				else if (strcmp(pCompression, "fl64") == 0)
				{
					m_OutputType = MEDIA_TYPE_PCM;
					m_PCMInfo.nFormat = PCM_FLOAT;
				}
				else if (strcmp(pCompression, "ALAW") == 0)
				{
					m_OutputType = MEDIA_TYPE_PCM;
					m_PCMInfo.nFormat = PCM_ALAW;
				}
			}
			else
			{
				m_OutputType = MEDIA_TYPE_PCM;
				m_PCMInfo.nFormat = PCM_LINEAR;
			}
		}
		else if (strcmp(str, "SSND") == 0)
		{
			m_StreamPos.nHeadPos = SourceReadBigEndian(4) + 4 + SourceGetPosition();
			m_StreamPos.nSize = nSize - 8;
			m_nOutputNum = 1;
		}
		if (nPos > SourceGetPosition())
			SourceSeekData(nPos - SourceGetPosition(), SEEK_CUR);  //skip rest of chunk
	}

	if (m_nOutputNum > 0)
	{
		m_PCMInfo.nSize = sizeof(PCMInfo);
		m_PCMInfo.nByteOrder = PCM_LITTLE_ENDIAN;
		m_PCMInfo.bSigned = true;
		m_nBytesPerSample = (m_PCMInfo.nBitsPerSample + 7) >> 3;
		SeekTime(0);
	}
}

CAIFFSource::~CAIFFSource()
{
	delete[] m_AIFFInfo.pCompressionName;
}

MediaType CAIFFSource::GetSourceType(void)
{
	return MEDIA_TYPE_AIFF;
}

bool CAIFFSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(AIFFInfo))
		return false;

	m_AIFFInfo.nSize = sizeof(AIFFInfo);
	memcpy(pInfo, &m_AIFFInfo, m_AIFFInfo.nSize);
	return true;
}

MediaType CAIFFSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : m_OutputType;
}

bool CAIFFSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	char str[50];
	int nLen;

	if (nIndex >= m_nOutputNum)
		return false;

	switch (m_OutputType)
	{
		case MEDIA_TYPE_PCM:
			if (pInfo->nSize >= sizeof(PCMInfo))
			{
				memcpy(pInfo, &m_PCMInfo, m_PCMInfo.nSize);
				return true;
			}
			break;
		case MEDIA_TYPE_UNKNOWN_AUDIO:
			sprintf(str, "Audio compression = %s", m_AIFFInfo.pCompressionType);
			nLen = strlen(str) + 1;
			if (pInfo->nSize >= sizeof(MediaInfo) + nLen)
			{
				pInfo->nSize = sizeof(MediaInfo) + nLen;
				strcpy(((TextInfo *)pInfo)->pText, str);
				return true;
			}
			break;
		default:
			break;
	}
	return false;
}

size_t CAIFFSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CAIFFSource::ReadData(int nIndex, void *pData, int nSize)
{
	int i, j;
	char *pBuffer, temp;

	if (nIndex >= m_nOutputNum)
		return 0;

	if (nSize > m_StreamPos.nSize - m_StreamPos.nRelPos)
		nSize = m_StreamPos.nSize - m_StreamPos.nRelPos;
	nSize = nSize / (m_nBytesPerSample * m_PCMInfo.nChannel) * (m_nBytesPerSample * m_PCMInfo.nChannel);
	nSize = SourceReadData(pData, nSize);
	if (pData != NULL)
	{
		pBuffer = (char *)pData;
		for (i = 0; i < nSize; i += m_nBytesPerSample)
		{
			for (j = (m_nBytesPerSample >> 1) - 1; j >= 0; j--)  //swap byte order
			{
				temp = pBuffer[j];
				pBuffer[j] = pBuffer[m_nBytesPerSample - j - 1];
				pBuffer[m_nBytesPerSample - j - 1] = temp;
			}
			pBuffer += m_nBytesPerSample;
		}
	}
	m_StreamPos.nRelPos += nSize;
	m_StreamPos.nAbsPos += nSize;
	return nSize;
}

bool CAIFFSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	bool bResult;
	
	if (nIndex >= m_nOutputNum)
		return false;
	
	if (nFrom == SEEK_CUR)
		nOffset += m_StreamPos.nRelPos;
	else if (nFrom == SEEK_END)
		nOffset += m_StreamPos.nSize;
	if (nOffset < 0)
	{
		nOffset = 0;
		bResult = false;
	}
	else if (nOffset > m_StreamPos.nSize)
	{
		nOffset = m_StreamPos.nSize;
		bResult = false;
	}
	else
		bResult = true;
	
	m_StreamPos.nRelPos = nOffset;
	m_StreamPos.nAbsPos = m_StreamPos.nHeadPos + m_StreamPos.nRelPos;
	SourceSeekData(m_StreamPos.nAbsPos, SEEK_SET);
	return bResult;
}

int CAIFFSource::SeekTime(int nTime)
{
	CMediaSource *pSource;

	if (m_ppOutputSources != NULL)
	{
		pSource = GetOutputSource(0);
		if (pSource != NULL)
			return pSource->SeekTime(nTime);
	}

	m_StreamPos.nRelPos = 0;
	m_StreamPos.nAbsPos = m_StreamPos.nHeadPos;
	SourceSeekData(m_StreamPos.nAbsPos, SEEK_SET);
	return 0;
}

int CAIFFSource::GetOutputTime(int nIndex)
{
	return 0;
}

int CAIFFSource::GetDuration(void)
{
	return m_nDuration;
}
