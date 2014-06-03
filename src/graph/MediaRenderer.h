#ifndef _MEDIARENDERER_H_
#define _MEDIARENDERER_H_

#include "MediaInfo.h"
#include "MediaRendererListener.h"

class CMediaRenderer
{
public:
	CMediaRenderer();
	virtual ~CMediaRenderer();
	virtual bool Open(MediaInfo *pInfo) = 0;
	virtual void Write(void *pBuffer, int nSize) = 0;
	virtual void Flush(bool bWait) = 0;
	virtual void Close(void) = 0;
	void SetListener(IMediaRendererListener *pListener);
protected:
	IMediaRendererListener *m_pListener;

	void FireRenderedEvent(void *pData, int nSize);
};

#endif
