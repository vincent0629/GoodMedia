#include "DIBRenderer.h"
#include <windef.h>

CDIBRenderer::CDIBRenderer() : CMediaRenderer()
{
	m_pBits = NULL;
}

CDIBRenderer::~CDIBRenderer()
{
	Close();
}

bool CDIBRenderer::Open(MediaInfo *pInfo)
{
	DIBRenderInfo *pRenderInfo;

	Close();

	if (pInfo->nSize < sizeof(DIBRenderInfo))
		return false;

	pRenderInfo = (DIBRenderInfo *)pInfo;
	if (pRenderInfo->nPixelFormat != PIXEL_RGB24 && pRenderInfo->nPixelFormat != PIXEL_RGB32)
		return false;

	m_nWidth = pRenderInfo->nWidth;
	m_nHeight = pRenderInfo->nHeight;
	m_nBytesPerPixel = pRenderInfo->nPixelFormat == PIXEL_RGB24? 3 : 4;
	m_hWindow = pRenderInfo->hwnd;

	memset(&m_BmpInfo, 0, sizeof(BITMAPINFO));
	m_BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_BmpInfo.bmiHeader.biWidth = m_nWidth;
	m_BmpInfo.bmiHeader.biHeight = -m_nHeight;
	m_BmpInfo.bmiHeader.biPlanes = 1;
	m_BmpInfo.bmiHeader.biBitCount = m_nBytesPerPixel << 3;
	m_BmpInfo.bmiHeader.biCompression = BI_RGB;

	m_nSize = ((m_nWidth * m_nBytesPerPixel + 3) & ~3) * m_nHeight;
	m_pBits = new unsigned char[m_nSize];
	memset(m_pBits, 0, m_nSize);
	return true;
}

void CDIBRenderer::Write(void *pBuffer, int nSize)
{
	int i, nBytesPerLine, nBytesPerLine4;
	RECT rect;
	HDC hdc;

	if (m_pBits == NULL)
		return;

	if (nSize > m_nSize)
		nSize = m_nSize;

	if (pBuffer != NULL && nSize >= m_nWidth * m_nHeight * m_nBytesPerPixel)
	{
		nBytesPerLine = m_nWidth * m_nBytesPerPixel;
		if ((nBytesPerLine & 3) == 0)
			memcpy(m_pBits, pBuffer, m_nHeight * nBytesPerLine);
		else
		{
			nBytesPerLine4 = (nBytesPerLine + 3) & ~3;
			for (i = m_nHeight - 1; i >= 0; i--)
				memcpy(m_pBits + i * nBytesPerLine4, (char *)pBuffer + i * nBytesPerLine, nBytesPerLine);
		}
	}

	GetClientRect(m_hWindow, &rect);
	hdc = GetDC(m_hWindow);
	StretchDIBits(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0, 0, m_nWidth, m_nHeight, m_pBits, &m_BmpInfo, DIB_RGB_COLORS, SRCCOPY);
	ReleaseDC(m_hWindow, hdc);
	ValidateRect(m_hWindow, NULL);

	FireRenderedEvent(pBuffer, nSize);
}

void CDIBRenderer::Flush(bool bWait)
{
	//memset(m_pBits, 0, m_nSize);
	//FireRenderedEvent(m_pBits, m_nSize);
}

void CDIBRenderer::Close(void)
{
	delete[] m_pBits;
	m_pBits = NULL;
}
