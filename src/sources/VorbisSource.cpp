#include "VorbisSource.h"
#include "ConvertUTF.h"
#include <string.h>

CVorbisSource::CVorbisSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i, nValue;
	char str[7], *temp;
	const UTF8 *pUTF8;
	UTF16 *pUTF16;

	memset(&m_VorbisInfo, 0, sizeof(VorbisInfo));

	str[6] = '\0';
	for (;;)
	{
		nValue = SourceReadData();  //type
		SourceReadData(str, 6);  //header
		if (strcmp(str, "vorbis") != 0)
			break;
		if (nValue == 1)  //identification
		{
			nValue = SourceReadLittleEndian(4);  //version
			m_VorbisInfo.nChannel = SourceReadData();  //channel
			m_VorbisInfo.nSampleRate = SourceReadLittleEndian(4);  //sample rate
			SourceSeekData(12, SEEK_CUR);  //bit rate
			SourceSeekData(2, SEEK_CUR);
		}
		else if (nValue == 3)  //comment
		{
			nValue = SourceReadLittleEndian(4);  //vendor length
			if (nValue > 0)
			{
				temp = new char[nValue + 1];
				SourceReadData(temp, nValue);  //vendor string
				temp[nValue] = '\0';
				m_VorbisInfo.pVendor = new wchar_t[nValue + 1];
				pUTF8 = (UTF8 *)temp;
				pUTF16 = (UTF16 *)m_VorbisInfo.pVendor;
				ConvertUTF8toUTF16(&pUTF8, pUTF8 + nValue, &pUTF16, pUTF16 + nValue, strictConversion);
				*pUTF16 = L'\0';
				delete[] temp;
			}

			m_VorbisInfo.nUserComment = SourceReadLittleEndian(4);  //user comment list length
			m_VorbisInfo.pUserComment = new wchar_t *[m_VorbisInfo.nUserComment];
			for (i = 0; i < m_VorbisInfo.nUserComment; i++)
			{
				nValue = SourceReadLittleEndian(4);  //length
				temp = new char[nValue + 1];
				SourceReadData(temp, nValue);
				temp[nValue] = '\0';
				m_VorbisInfo.pUserComment[i] = new wchar_t[nValue + 1];
				pUTF8 = (UTF8 *)temp;
				pUTF16 = (UTF16 *)m_VorbisInfo.pUserComment[i];
				ConvertUTF8toUTF16(&pUTF8, pUTF8 + nValue, &pUTF16, pUTF16 + nValue, strictConversion);
				*pUTF16 = L'\0';
				delete[] temp;
			}
		}
		else if (nValue == 5)  //setup
		{
		}
		else
			break;

		m_nOutputNum = 1;
	}
}

CVorbisSource::~CVorbisSource()
{
}

MediaType CVorbisSource::GetSourceType(void)
{
	return MEDIA_TYPE_VORBIS;
}

bool CVorbisSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(VorbisInfo))
		return false;

	m_VorbisInfo.nSize = sizeof(VorbisInfo);
	memcpy(pInfo, &m_VorbisInfo, m_VorbisInfo.nSize);
	return true;
}

MediaType CVorbisSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_VORBIS;
}

bool CVorbisSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CVorbisSource::GetSize(int nIndex)
{
	return 0;
}

size_t CVorbisSource::GetPosition(int nIndex)
{
	return 0;
}

int CVorbisSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	return 0;
}

bool CVorbisSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;
}

int CVorbisSource::GetOutputTime(int nIndex)
{
	return 0;
}

int CVorbisSource::GetDuration(void)
{
	return 0;
}

int CVorbisSource::SeekTime(int nTime)
{
	return 0;
}
