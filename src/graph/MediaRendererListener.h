#ifndef _MEDIARENDERERLISTENER_H_
#define _MEDIARENDERERLISTENER_H_

class CMediaRenderer;

class IMediaRendererListener
{
public:
	virtual void MediaRendered(CMediaRenderer *pRenderer, void *pData, int nSize) = 0;
};

#endif
