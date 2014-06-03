#include "TextRenderer.h"
#include <windef.h>

CTextRenderer::CTextRenderer() : CMediaRenderer()
{
	m_pBits = NULL;
}

CTextRenderer::~CTextRenderer()
{
	Close();
}

bool CTextRenderer::Open(MediaInfo *pInfo)
{
	TextRenderInfo *pRenderInfo;

	Close();

	if (pInfo->nSize < sizeof(TextRenderInfo))
		return false;

	pRenderInfo = (TextRenderInfo *)pInfo;
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

void CTextRenderer::Write(void *pBuffer, int nSize)
{
	HDC hdc;
	TextRenderData *pData;

	if (nSize < sizeof(TextRenderData))
		return;

	pData = (TextRenderData *)pBuffer;
	hdc = GetDC(m_hWindow);
	SetTextColor(hdc, pData->nColor);
	SetBkMode(hdc, TRANSPARENT);
	TextOut(hdc, pData->x, pData->y, pData->pText, _tcslen(pData->pText));
	ReleaseDC(m_hWindow, hdc);

	FireRenderedEvent((void *)pData->pText, _tcslen(pData->pText));
}

void CTextRenderer::Flush(bool bWait)
{
	memset(m_pBits, 0, m_nSize);
	FireRenderedEvent(m_pBits, m_nSize);
}

void CTextRenderer::Close(void)
{
	delete[] m_pBits;
	m_pBits = NULL;
}
