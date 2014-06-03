#ifndef _TEXTRENDERER_H_
#define _TEXTRENDERER_H_

#include "MediaRenderer.h"
#include <windows.h>
#include <wingdi.h>

typedef struct _GDIRenderInfo : VideoRenderInfo
{
	HWND hwnd;
} TextRenderInfo;

typedef struct
{
	int x, y;
	unsigned long nColor;
	const TCHAR *pText;
} TextRenderData;

class CTextRenderer : public CMediaRenderer
{
public:
	CTextRenderer();
	~CTextRenderer();
	bool Open(MediaInfo *pInfo);
	void Write(void *pBuffer, int nSize);
	void Flush(bool bWait);
	void Close(void);
private:
	HWND m_hWindow;
	int m_nWidth, m_nHeight, m_nBytesPerPixel;
	BITMAPINFO m_BmpInfo;
	unsigned char *m_pBits;
	int m_nSize;
};

#endif
