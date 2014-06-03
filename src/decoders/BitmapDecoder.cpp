#include "BitmapDecoder.h"
#include "BitmapInfo.h"
#include <string.h>

static int decodeRGB(unsigned char *pData, int nTotal, unsigned char *pColorTable, int nWidth, int nHeight, int nBytesPerPixel, unsigned char *pFrame)
{
	int nBytesPerLine;
	unsigned char *pLine;
	int x, y;
	unsigned int nColor;

	nBytesPerLine = nWidth * 3;
	pLine = pFrame + (nHeight - 1) * nBytesPerLine;
	if (nBytesPerPixel == 1)
	{
		for (y = nHeight - 1; y >= 0; y--)
		{
			for (x = 0; x < nWidth; x++)
			{
				memcpy(pLine + x * 3, pColorTable + *pData * 4, 3);
				++pData;
			}
			pLine -= nBytesPerLine;
		}
	}
	else if (nBytesPerPixel == 2)
	{
		for (y = nHeight - 1; y >= 0; y--)
		{
			for (x = 0; x < nWidth; x++)
			{
				nColor = *pData | (*(pData + 1) << 8);  //R:G:B = 5:5:5
				pLine[x * 3 + 0] = (nColor << 3) & 0xF8;
				pLine[x * 3 + 1] = (nColor >> 2) & 0xF8;
				pLine[x * 3 + 2] = (nColor >> 7) & 0xF8;
				pData += 2;
			}
			pLine -= nBytesPerLine;
		}
	}
	else if (nBytesPerPixel == 3)
	{
		for (y = nHeight - 1; y >= 0; y--)
		{
			memcpy(pLine, pData, nBytesPerLine);
			pLine -= nBytesPerLine;
			pData += (nBytesPerLine + 3) & ~3;
		}
	}
	else if (nBytesPerPixel == 4)  //not tested
	{
		for (y = nHeight - 1; y >= 0; y--)
		{
			for (x = 0; x < nWidth; x++)
			{
				pLine[x * 3 + 0] = pData[0];
				pLine[x * 3 + 1] = pData[1];
				pLine[x * 3 + 2] = pData[2];
				pData += 4;
			}
			pLine -= nBytesPerLine;
		}
	}
	return nWidth * nHeight * 3;
}

static int decodeRLE8(unsigned char *pData, int nTotal, unsigned char *pColorTable, int nWidth, int nHeight, int nBytesPerPixel, unsigned char *pFrame)
{
	int nBytesPerLine, nCount, nColor;
	int i, x, y;
	unsigned char *pLine, *pColor;

	nBytesPerLine = nWidth * 3;
	x = 0;
	y = nHeight - 1;
	pLine = pFrame + y * nBytesPerLine;

	while (nTotal > 0)
	{
		nCount = *(pData++);
		nColor = *(pData++);
		nTotal -= 2;
		if (nCount == 0)
		{
			if (nColor == 0)  //end of line
			{
				x = 0;
				if (--y < 0)
					break;
				pLine = pFrame + y * nBytesPerLine;
			}
			else if (nColor == 1)  //end of bitmap
				break;
			else if (nColor == 2)  //delta
			{
				x += *(pData++);
				y -= *(pData++);
				if (x < 0 || x > nWidth || y < 0 || y > nHeight)
					break;
				pLine = pFrame + y * nBytesPerLine;
				nTotal -= 2;
			}
			else
			{
				nCount = nColor;
				for (i = 0; i < nCount; i++)
				{
					nColor = *(pData++);
					memcpy(pLine + x * 3, pColorTable + nColor * 4, 3);
					if (++x > nWidth)
					{
						x = 0;
						if (--y < 0)
						{
							nCount = 0;
							nTotal = 0;
							break;
						}
						pLine = pFrame + y * nBytesPerLine;
					}
				}
				if (nCount & 1)
				{
					++pData;  //align
					++nCount;
				}
				nTotal -= nCount;
			}
		}
		else
		{
			pColor = pColorTable + nColor * 4;
			for (i = 0; i < nCount && x < nWidth; i++, x++)
				memcpy(pLine + x * 3, pColor, 3);
		}
	}
	return nWidth * nHeight * 3;
}

static int decodeRLE4(unsigned char *pData, int nTotal, unsigned char *pColorTable, int nWidth, int nHeight, int nBytesPerPixel, unsigned char *pFrame)
{
	return 0;
}

static int decodeYUY2(unsigned char *pData, int nTotal, int nWidth, int nHeight, int nBytesPerPixel, unsigned char *pFrame)
{
	int i;

	for (i = 0; i < nWidth * nHeight; i++)
	{
		pFrame[i * 3] = pData[i * 2];
		pFrame[i * 3 + 1] = pFrame[i * 3];
		pFrame[i * 3 + 2] = pFrame[i * 3]; 
	}
	return nWidth * nHeight * 3;
}

CBitmapDecoder::CBitmapDecoder()
{
}

CBitmapDecoder::~CBitmapDecoder()
{
}

bool CBitmapDecoder::SetInputInfo(MediaType type, MediaInfo *pInfo)
{
	BitmapInfo *pBitmapInfo;

	if (pInfo->nSize < sizeof(BitmapInfo))
		return false;

	pBitmapInfo = (BitmapInfo *)pInfo;
	if (pBitmapInfo->nCompression > 2 && pBitmapInfo->nCompression != 0x32595559)  //not support
		return false;

	m_nWidth = pBitmapInfo->nWidth;
	m_nHeight = pBitmapInfo->nHeight;
	m_nBytesPerPixel = pBitmapInfo->nBitsPerPixel >> 3;
	m_nCompression = pBitmapInfo->nCompression;
	m_pColorTable = pBitmapInfo->pColorTable;
	return true;
}

bool CBitmapDecoder::GetOutputInfo(MediaInfo *pInfo)
{
	VideoRenderInfo *pVideoInfo;

	if (pInfo->nSize < sizeof(VideoRenderInfo))
		return false;

	pVideoInfo = (VideoRenderInfo *)pInfo;
	pVideoInfo->nSize = sizeof(VideoRenderInfo);
	pVideoInfo->nWidth = m_nWidth;
	pVideoInfo->nHeight = m_nHeight;
	pVideoInfo->fPixelAspectRatio = 1.0f;
	pVideoInfo->nPixelFormat = PIXEL_RGB24;
	return true;
}

int CBitmapDecoder::GetInputSize(void)
{
	return ((m_nWidth * m_nBytesPerPixel + 3) & ~3) * m_nHeight;
}

int CBitmapDecoder::GetOutputSize(void)
{
	return m_nWidth * m_nHeight * 3;
}

int CBitmapDecoder::Decode(unsigned char *pInData, int nInSize, void *pOutData)
{
	if (m_nCompression == 0)  //RGB
		return decodeRGB(pInData, nInSize, m_pColorTable, m_nWidth, m_nHeight, m_nBytesPerPixel, (unsigned char *)pOutData);
	else if (m_nCompression == 1)  //RLE8
		return decodeRLE8(pInData, nInSize, m_pColorTable, m_nWidth, m_nHeight, m_nBytesPerPixel, (unsigned char *)pOutData);
	else if (m_nCompression == 2)  //RLE4
		return decodeRLE4(pInData, nInSize, m_pColorTable, m_nWidth, m_nHeight, m_nBytesPerPixel, (unsigned char *)pOutData);
	else if (m_nCompression == 0x32595559)  //YUY2
		return decodeYUY2(pInData, nInSize, m_nWidth, m_nHeight, m_nBytesPerPixel, (unsigned char *)pOutData);
	return 0;
}
