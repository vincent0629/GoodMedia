#include "PNGSource.h"
#include <string.h>

CPNGSource::CPNGSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	PNGColorType pColorTypeTable[] = {PNG_GREYSCALE, PNG_UNKNOWNCOLOR, PNG_TRUECOLOR, PNG_INDEXEDCOLOR, PNG_GREYSCALEWITHALPHA, PNG_UNKNOWNCOLOR, PNG_TRUECOLORWITHALPHA};
	long nValue, nLen;
	char str[5];
	int nState;
	size_t nPos;

	nValue = SourceReadBigEndian(4);
	if (nValue != 0x89504E47)  //PNG signature
		return;
	nValue = SourceReadBigEndian(4);
	if (nValue != 0x0D0A1A0A)  //PNG signature
		return;

	m_PNGInfo.pColorTable = NULL;
	m_PNGInfo.nColorUsed = 0;
	str[4] = '\0';
	nState = 1;
	while (nState > 0)
	{
		nLen = SourceReadBigEndian(4);  //chunk length
		nPos = SourceGetPosition() + nLen + 8;
		SourceReadData(str, 4);  //chunk type
		switch (nState)
		{
			case 1:
				if (strcmp(str, "IHDR") == 0)
				{
					m_PNGInfo.nWidth = SourceReadBigEndian(4);
					m_PNGInfo.nHeight = SourceReadBigEndian(4);
					m_PNGInfo.nBitsPerPixel = SourceReadData();
					m_PNGInfo.nColorType = pColorTypeTable[SourceReadData()];
					nState = 2;
				}
				else
					nState = -1;
				break;
			case 2:
			case 3:
				if (strcmp(str, "IDAT") == 0)
				{
					m_Pos.nHeadPos = SourceGetPosition();
					m_Pos.nSize = nLen;
					nState = 3;
				}
				else if (strcmp(str, "PLTE") == 0)
				{
					m_PNGInfo.nColorUsed = nLen / 3;
					m_PNGInfo.pColorTable = new unsigned char[nLen];
					SourceReadData(m_PNGInfo.pColorTable, nLen);
				}
				else if (strcmp(str, "IEND") == 0)
					nState = nState == 2? -1 : 0;
				break;
		}  //siwtch

		SourceSeekData(nPos, SEEK_SET);  //skip to next chunk
	}  //while

	if (nState == 0)
	{
		m_nOutputNum = 1;
		SeekTime(0);
	}
}

CPNGSource::~CPNGSource()
{
	if (m_nOutputNum > 0)
		delete[] m_PNGInfo.pColorTable;
}

MediaType CPNGSource::GetSourceType(void)
{
	return MEDIA_TYPE_PNG;
}

bool CPNGSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(PNGInfo))
		return false;

	m_PNGInfo.nSize = sizeof(PNGInfo);
	memcpy(pInfo, &m_PNGInfo, m_PNGInfo.nSize);
	return true;
}

MediaType CPNGSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_PNG;
}

bool CPNGSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CPNGSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_Pos.nRelPos;
}

int CPNGSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;
	if (nSize > m_Pos.nSize - m_Pos.nRelPos)
		nSize = m_Pos.nSize - m_Pos.nRelPos;
	nSize = SourceReadData(pData, nSize);
	m_Pos.nRelPos += nSize;
	return nSize;
}

bool CPNGSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	if (nIndex >= m_nOutputNum)
		return false;

	if (nFrom == SEEK_CUR)
		nOffset += m_Pos.nRelPos;
	else if (nFrom == SEEK_END)
		nOffset += m_Pos.nSize;
	if (nOffset < 0 || nOffset > m_Pos.nSize)
		return false;

	m_Pos.nRelPos = nOffset;
	SourceSeekData(m_Pos.nHeadPos + m_Pos.nRelPos, SEEK_SET);
	return true;
}

int CPNGSource::GetDuration(void)
{
	return 0;
}

int CPNGSource::SeekTime(int nTime)
{
	SourceSeekData(m_Pos.nHeadPos, SEEK_SET);
	m_Pos.nRelPos = 0;
	return 0;
}
