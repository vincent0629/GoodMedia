#ifndef _DIBRENDERER_H_
#define _DIBRENDERER_H_

#include "MediaRenderer.h"
#include <windows.h>
#include <wingdi.h>

typedef struct _DIBRenderInfo : VideoRenderInfo
{
	HWND hwnd;
} DIBRenderInfo;

class CDIBRenderer : public CMediaRenderer
{
public:
	CDIBRenderer();
	~CDIBRenderer();
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
