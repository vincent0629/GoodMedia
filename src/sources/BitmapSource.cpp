#include "BitmapSource.h"
#include <string.h>

CBitmapSource::CBitmapSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	memset(&m_StreamPos, 0, sizeof(StreamPos));
	if (SourceReadData() != 'B')
		return;
	if (SourceReadData() != 'M')
		return;

	SourceSeekData(8, SEEK_CUR);
	m_StreamPos.nHeadPos = SourceReadLittleEndian(4);  //bitmap data offset
	SourceSeekData(4, SEEK_CUR);  //header size
	m_BitmapInfo.nWidth = SourceReadLittleEndian(4);  //width
	m_BitmapInfo.nHeight = SourceReadLittleEndian(4);  //height
	SourceSeekData(2, SEEK_CUR);  //planes
	m_BitmapInfo.nBitsPerPixel = SourceReadLittleEndian(2);  //bits per pixel
	m_BitmapInfo.nCompression = SourceReadLittleEndian(4);  //compression
	m_StreamPos.nSize = SourceReadLittleEndian(4);  //bitmap data size
	SourceSeekData(8, SEEK_CUR);
	m_BitmapInfo.nColorUsed = SourceReadLittleEndian(4);  //color used
	SourceSeekData(4, SEEK_CUR);  //important colors
	if (m_BitmapInfo.nColorUsed != 0)
	{
		m_BitmapInfo.pColorTable = new unsigned char[m_BitmapInfo.nColorUsed * 4];
		SourceReadData(m_BitmapInfo.pColorTable, m_BitmapInfo.nColorUsed * 4);  //color table
	}
	else
		m_BitmapInfo.pColorTable = NULL;
	if (m_BitmapInfo.nHeight < 0)
	{
		m_BitmapInfo.nHeight = -m_BitmapInfo.nHeight;
		m_BitmapInfo.bTopDown = true;
	}
	else
		m_BitmapInfo.bTopDown = false;
	if (m_StreamPos.nSize == 0)
		m_StreamPos.nSize = SourceGetSize() - SourceGetPosition();

	m_nOutputNum = 1;
	SeekTime(0);
}

CBitmapSource::~CBitmapSource()
{
	if (m_nOutputNum > 0)
		delete[] m_BitmapInfo.pColorTable;
}

MediaType CBitmapSource::GetSourceType(void)
{
	return MEDIA_TYPE_BITMAP;
}

bool CBitmapSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(BitmapInfo))
		return false;

	m_BitmapInfo.nSize = sizeof(BitmapInfo);
	memcpy(pInfo, &m_BitmapInfo, m_BitmapInfo.nSize);
	return true;
}

MediaType CBitmapSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_BITMAP;
}

bool CBitmapSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CBitmapSource::GetSize(int nIndex)
{
	return m_StreamPos.nSize;
}

size_t CBitmapSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nRelPos;
}

int CBitmapSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	if (m_StreamPos.nRelPos + nSize >= m_StreamPos.nSize)
		nSize = m_StreamPos.nSize - m_StreamPos.nRelPos;
	if (nSize > 0)
	{
		nSize = SourceReadData(pData, nSize);
		m_StreamPos.nRelPos += nSize;
	}
	return nSize;
}

bool CBitmapSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;
}

int CBitmapSource::GetDuration(void)
{
	return 0;
}

int CBitmapSource::SeekTime(int nTime)
{
	SourceSeekData(m_StreamPos.nHeadPos, SEEK_SET);
	m_StreamPos.nRelPos = 0;
	return 0;
}
