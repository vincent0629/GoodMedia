#include "PESPacketSource.h"
#include "MPEGProgramInfo.h"
#include <string.h>

CPESPacketSource::CPESPacketSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	MPEGProgramInfo info;
	int nPacketLen, nValue;
	int sampleRateTable[] = {48000, 96000, 192000, 0};
	int bitsTable[] = {16, 20, 24, 0};

	if (pSource->GetSourceType() == MEDIA_TYPE_MPEGPROGRAM)
	{
		info.nSize = sizeof(MPEGProgramInfo);
		if (pSource->GetSourceInfo(&info))
			m_nMPEGVersion = info.nVersion;
		else
			return;
	}
	else if (pSource->GetSourceType() == MEDIA_TYPE_MPEGTRANSPORT)
		m_nMPEGVersion = 2;
	else
		return;
	m_OutputType = MEDIA_TYPE_UNKNOWN;
	nPacketLen = NextPacket(NULL);
	if (nPacketLen == -1)
		return;

	m_nOutputNum = 1;
	if (m_Info.nStreamID >= 0xC0 && m_Info.nStreamID <= 0xCF)
		m_OutputType = MEDIA_TYPE_MPEGAUDIO;
	else if (m_Info.nStreamID >= 0xD8 && m_Info.nStreamID <= 0xDF)
		m_OutputType = MEDIA_TYPE_AAC;
	else if (m_Info.nStreamID >= 0xE0 && m_Info.nStreamID <= 0xEF)
		m_OutputType = MEDIA_TYPE_MPEGVIDEO;
	else if (m_Info.nStreamID == 0xBD)  //private stream 1
	{
		if (m_Info.nStreamSubID >= 0x20 && m_Info.nStreamSubID <= 0x3F)
			m_OutputType = MEDIA_TYPE_SUBPICTURE;
		else if (m_Info.nStreamSubID >= 0x80 && m_Info.nStreamSubID <= 0x87)
			m_OutputType = MEDIA_TYPE_AC3;
		else if (m_Info.nStreamSubID >= 0x88 && m_Info.nStreamSubID <= 0x9F)
			m_OutputType = MEDIA_TYPE_DTS;
		else if (m_Info.nStreamSubID >= 0xA0 && m_Info.nStreamSubID <= 0xA7)
		{
			m_OutputType = MEDIA_TYPE_PCM;
			SourceSeekData(3, SEEK_CUR);
			nValue = SourceReadBigEndian(2);
			m_PCMInfo.nSampleRate = sampleRateTable[(nValue & 0x30) >> 4];
			m_PCMInfo.nBitsPerSample = bitsTable[(nValue & 0xC0) >> 6];
			m_PCMInfo.nChannel = (nValue & 0x07) + 1;
			SourceReadData();  //dynamic range control
			m_PCMInfo.nFormat = PCM_LINEAR;
			m_PCMInfo.nByteOrder = PCM_BIG_ENDIAN;
			m_PCMInfo.bSigned = true;
		}
		else if (m_Info.nStreamSubID >= 0xB0 && m_Info.nStreamSubID <= 0xB7)
			m_OutputType = MEDIA_TYPE_MLP;
		else if (m_Info.nStreamSubID >= 0xB8 && m_Info.nStreamSubID <= 0xBF)
			m_OutputType = MEDIA_TYPE_WMA;
		else if (m_Info.nStreamSubID >= 0xC0 && m_Info.nStreamSubID <= 0xC7)
			m_OutputType = MEDIA_TYPE_DDPLUS;
	}
	SeekTime(0);
}

CPESPacketSource::~CPESPacketSource()
{
}

MediaType CPESPacketSource::GetSourceType(void)
{
	return MEDIA_TYPE_PESPACKET;
}

bool CPESPacketSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(PESPacketInfo))
		return false;

	m_Info.nSize = sizeof(PESPacketInfo);
	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

MediaType CPESPacketSource::GetOutputType(int nIndex)
{
	return m_OutputType;
}

bool CPESPacketSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	if (nIndex >= m_nOutputNum || m_OutputType != MEDIA_TYPE_PCM || pInfo->nSize < sizeof(PCMInfo))
		return false;

	m_PCMInfo.nSize = sizeof(PCMInfo);
	memcpy(pInfo, &m_PCMInfo, m_PCMInfo.nSize);
	return true;
}

size_t CPESPacketSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CPESPacketSource::ReadData(int nIndex, void *pData, int nSize)
{
	int nPacketLen, nSkip;
	size_t nPTS;

	if (nIndex >= m_nOutputNum)
		return 0;

	if (m_StreamPos.nRelPos >= m_StreamPos.nSize)
	{
		nPTS = 0;
		nPacketLen = NextPacket(&nPTS);
		if (nPacketLen == -1)
			return 0;
		if (m_Info.nStreamID == 0xBD)  //private stream 1
		{
			if (m_OutputType == MEDIA_TYPE_AC3)
				nSkip = 3;
			else if (m_OutputType == MEDIA_TYPE_DTS)
				nSkip = 3;
			else if (m_OutputType == MEDIA_TYPE_PCM)
				nSkip = 6;
			else if (m_OutputType == MEDIA_TYPE_MLP)
				nSkip = 4;
			else if (m_OutputType == MEDIA_TYPE_DDPLUS)
				nSkip = 3;
			else
				nSkip = 0;
			if (nSkip > 0)
			{
				SourceSeekData(nSkip, SEEK_CUR);
				nPacketLen -= nSkip;
			}
		}
		m_StreamPos.nSize += nPacketLen == 0? 0x10000 : nPacketLen;
		if (nPTS != 0)
			m_StreamPos.nTime = (int)(nPTS / 90);
	}

	if (nSize > m_StreamPos.nSize - m_StreamPos.nRelPos)
		nSize = m_StreamPos.nSize - m_StreamPos.nRelPos;
	nSize = SourceReadData(pData, nSize);
	m_StreamPos.nRelPos += nSize;
	return nSize;
}

bool CPESPacketSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	int nSize, n;

	if (nIndex >= m_nOutputNum)
		return false;

	if (nFrom == SEEK_CUR && nOffset >= 0)
	{
		nSize = 0;
		while (nSize < nOffset)
		{
			n = ReadData(nIndex, NULL, nOffset - nSize);
			nSize += n;
		}
		return true;
	}
	else if (nFrom == SEEK_SET && nOffset == 0)
	{
		m_StreamPos.nRelPos = 0;
		m_StreamPos.nSize = 0;
		return SourceSeekData(0, SEEK_SET);
	}
	return false;
}

int CPESPacketSource::GetOutputTime(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nTime;
}

int CPESPacketSource::GetDuration(void)
{
	return 0;
}

int CPESPacketSource::SeekTime(int nTime)
{
	CMediaSource *pSource;

	if (m_ppOutputSources != NULL)
	{
		pSource = GetOutputSource(0);
		if (pSource != NULL)
			pSource->SeekTime(0);
	}
	m_StreamPos.nRelPos = 0;
	m_StreamPos.nSize = 0;
	m_StreamPos.nTime = 0;
	SourceSeekData(0, SEEK_SET);
	return 0;
}

int CPESPacketSource::NextPacket(size_t *pnPTS)
{
	unsigned long nSync;
	int i;
	int nPacketLen, nHeaderLen;
	int nFlag, nValue;

	nSync = SourceReadBigEndian(4);
	for (i = 0x10000; i > 0; i--)
	{
		if (nSync >= 0x000001BC && nSync <= 0x000001FF)
			break;

		nValue = SourceReadData();
		if (nValue == -1)
			return -1;
		nSync = (nSync << 8) | nValue;
	}
	if (i == 0)
		return -1;

	m_Info.nStreamID = nSync & 0xFF;
	nPacketLen = SourceReadBigEndian(2);
	if (nPacketLen > 0)
		nPacketLen += SourceGetPosition();

	if (m_nMPEGVersion == 1)  //MPEG-1
	{
		if (m_Info.nStreamID != 0xBF)
		{
			do
			{
				nFlag = SourceReadData();
			} while (nFlag & 0x80);
			if ((nFlag & 0xC0) == 0x40)  //STD buffer
			{
				SourceReadData();
				nFlag = SourceReadData();
			}
			if ((nFlag & 0xE0) == 0x20)  //PTS
			{
				if (pnPTS == NULL)
					SourceSeekData(4, SEEK_CUR);
				else
				{
					*pnPTS = nFlag;
					*pnPTS = (*pnPTS & 0x0E) << 29;
					nValue = SourceReadBigEndian(4);
					*pnPTS |= (nValue & 0xFFFE0000) >> 2;
					*pnPTS |= (nValue & 0xFFFE) >> 1;
				}
			}
			if ((nFlag & 0xF0) == 0x30)  //DTS
				SourceSeekData(5, SEEK_CUR);
		}
	}
	else  //MPEG-2
	{
		if (m_Info.nStreamID != 0xBC && m_Info.nStreamID != 0xBE && m_Info.nStreamID != 0xBF && m_Info.nStreamID != 0xF0 && m_Info.nStreamID != 0xF1 && m_Info.nStreamID != 0xFF && m_Info.nStreamID != 0xF2 && m_Info.nStreamID != 0xF8)
		{
			SourceReadData();
			nFlag = SourceReadData();
			nHeaderLen = SourceReadData();
			nHeaderLen += SourceGetPosition();
			if (nFlag & 0x80)  //PTS
			{
				if (pnPTS == NULL)
					SourceSeekData(5, SEEK_CUR);
				else
				{
					nValue = SourceReadData();
					if (((nValue >> 4) & 0x03) == (nFlag >> 6))
					{
						*pnPTS = nValue;
						*pnPTS = (*pnPTS & 0x0E) << 29;
						nValue = SourceReadBigEndian(4);
						*pnPTS |= (nValue & 0xFFFE0000) >> 2;
						*pnPTS |= (nValue & 0xFFFE) >> 1;
					}
				}
			}
			SourceSeekData(nHeaderLen - SourceGetPosition(), SEEK_CUR);  //skip rest of header

			if (m_Info.nStreamID == 0xBD)  //private stream 1
				m_Info.nStreamSubID = SourceReadData();
		}  //if
	}  //else
	return nPacketLen == 0? 0 : nPacketLen - SourceGetPosition();
}
