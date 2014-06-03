#include "AC3Source.h"
#include <string.h>
#include <stdint.h>

CAC3Source::CAC3Source(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i, j;
	int nFramePos[4], nFrameSize[4];
	bool bDone;
	int nValue, acmod;
	int channelTable[] = {2, 1, 2, 3, 3, 4, 4, 5};

	m_SourceType = MEDIA_TYPE_AC3;

	bDone = false;
	for (i = 0; i < 4 && !bDone; i++)
	{
		nFrameSize[i] = NextSync(true);
		if (nFrameSize[i] == 0)
			break;
		nFramePos[i] = SourceGetPosition() - HEADER_SIZE;

		for (j = 0; j < i; j++)
			if (nFramePos[j] + nFrameSize[j] == nFramePos[i])
			{
				m_StreamPos.nHeadPos = nFramePos[j];
				m_nFrameSize = nFrameSize[j];
				bDone = true;
				break;
			}
	}
	if (!bDone)
		return;

	if (m_SourceType == MEDIA_TYPE_AC3)
	{
		nValue = m_bSwapByte? (m_pHeader[7] << 8) | m_pHeader[6] : (m_pHeader[6] << 8) | m_pHeader[7];
		acmod = (nValue >> 13) & 7;
		m_AC3Info.nChannel = channelTable[acmod];
		i = 0;
		if ((acmod & 0x01) != 0 && acmod != 0x01)  //cmixlev
			i += 2;
		if ((acmod & 0x04) != 0)  //surmixlev
			i += 2;
		if (acmod == 0x02)  //dsurmod
			i += 2;
		if (((nValue >> (12 - i)) & 1) != 0)  //lfeon
			++m_AC3Info.nChannel;
	}
	else
	{
		acmod = (m_pHeader[m_bSwapByte? 5 : 4] >> 1) & 0x07;
		m_AC3Info.nChannel = channelTable[acmod];
		if ((m_pHeader[m_bSwapByte? 5 : 4] & 1) != 0)  //lfeon
			++m_AC3Info.nChannel;
	}
	m_nOutputNum = 1;

	SeekTime(0);
}

CAC3Source::~CAC3Source()
{
}

MediaType CAC3Source::GetSourceType(void)
{
	return m_SourceType;
}

bool CAC3Source::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(AudioInfo))
		return false;

	m_AC3Info.nSize = sizeof(AudioInfo);
	memcpy(pInfo, &m_AC3Info, m_AC3Info.nSize);
	return true;
}

MediaType CAC3Source::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : m_SourceType;
}

bool CAC3Source::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CAC3Source::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CAC3Source::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	nSize = NextSync(false);
	if (nSize == 0)
		return 0;

	if (pData != NULL)
	{
		memcpy(pData, m_pHeader, HEADER_SIZE);
		SourceReadData((char *)pData + HEADER_SIZE, nSize - HEADER_SIZE);
	}
	else
		SourceSeekData(nSize - HEADER_SIZE, SEEK_CUR);
	m_StreamPos.nRelPos += nSize;
	return nSize;
}

bool CAC3Source::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CAC3Source::GetOutputTime(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nTime;
}

int CAC3Source::GetDuration(void)
{
	int nFrameNum;

	if (m_nOutputNum == 0)
		return 0;

	nFrameNum = (SourceGetSize() - m_StreamPos.nHeadPos) / m_nFrameSize;
	return (int)((int64_t)nFrameNum * 1536 * 1000 / m_AC3Info.nSampleRate);
}

int CAC3Source::SeekTime(int nTime)
{
	if (m_nOutputNum > 0)
	{
		SourceSeekData(m_StreamPos.nHeadPos, SEEK_SET);
		m_StreamPos.nAbsPos = m_StreamPos.nHeadPos;
		m_StreamPos.nRelPos = m_StreamPos.nSize = 0;
		m_StreamPos.nTime = 0;
		m_nFrameIndex = 0;
	}
	return 0;
}

int CAC3Source::NextSync(bool bSearch)
{
	static int sampleRateTable[3] = {48000, 44100, 32000};
	static int frameSizeTable[3][38] = {
		{64, 64, 80, 80, 96, 96, 112, 112, 128, 128, 160, 160, 192, 192, 224, 224, 256, 256, 320, 320, 384, 384, 448, 448, 512, 512, 640, 640, 768, 768, 896, 896, 1024, 1024, 1152, 1152, 1280, 1280},
		{69, 70, 87, 88, 104, 105, 121, 122, 139, 140, 174, 175, 208, 209, 243, 244, 278, 279, 348, 349, 417, 418, 487, 488, 557, 558, 696, 697, 835, 836, 975, 976, 1114, 1115, 1253, 1254, 1393, 1394},
		{96, 96, 120, 120, 144, 144, 168, 168, 192, 192, 240, 240, 288, 288, 336, 336, 384, 384, 480, 480, 576, 576, 672, 672, 768, 768, 960, 960, 1152, 1152, 1344, 1344, 1536, 1536, 1728, 1728, 1920, 1920}
	};
	int i, nValue;
	int nSize;

	if (SourceReadData(m_pHeader, 1) < 1)
		return 0;

	for (i = bSearch? 8192 : 1; i != 0; i--)  //8192 is max frame size
	{
		if (SourceReadData(m_pHeader + 1, 1) < 1)
			return 0;
		if (m_pHeader[0] == 0x0B && m_pHeader[1] == 0x77)  //sync word
		{
			m_bSwapByte = false;
			break;
		}
		else if (m_pHeader[0] == 0x77 && m_pHeader[1] == 0x0B)  //sync word
		{
			m_bSwapByte = true;
			break;
		}
		m_pHeader[0] = m_pHeader[1];
	}
	if (i == 0)
		return 0;

	SourceReadData(m_pHeader + 2, HEADER_SIZE - 2);  //AC3(CRC1), DD+(strmtyp:2 substreamid:3 frmsiz:11)
	                                                 //AC3(fscod:2 frmsizecod:6 bsid:5 bsmod:3 ...), DD+(fscod:2 fscod2/numblkscod:2 acmod:3 lfeon:1 bsid:5 ...)
	nValue = m_pHeader[m_bSwapByte? 4 : 5] >> 3;  //bsid
	m_SourceType = nValue <= 8? MEDIA_TYPE_AC3 : MEDIA_TYPE_DDPLUS;
	nValue = m_pHeader[m_bSwapByte? 5 : 4] >> 6;  //fscod
	if (nValue == 3)  //invalid
		return 0;
	m_AC3Info.nSampleRate = sampleRateTable[nValue];
	if (m_SourceType == MEDIA_TYPE_AC3)
		nSize = frameSizeTable[nValue][m_pHeader[m_bSwapByte? 5 : 4] & 0x3F] << 1;
	else
		nSize = ((((m_pHeader[m_bSwapByte? 3 : 2] & 0x07) << 8) | m_pHeader[m_bSwapByte? 2 : 3]) + 1) << 1;

	m_StreamPos.nTime = (int)((int64_t)m_nFrameIndex * 1536 * 1000 / m_AC3Info.nSampleRate);
	++m_nFrameIndex;
	return nSize;
}
