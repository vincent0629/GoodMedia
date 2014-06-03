#ifndef _MEDIASOURCEFACTORY_H_
#define _MEDIASOURCEFACTORY_H_

#include "MediaSource.h"

typedef CMediaSource *(*MEDIASOURCE_CREATE_FUNC)(MediaType type, CMediaSource *pSource, int nIndex);

class CMediaSourceFactory
{
public:
	static void Register(MediaType type, MEDIASOURCE_CREATE_FUNC func);
	static CMediaSource *Create(MediaType type, CMediaSource *pSource, int nIndex);
private:
	static MEDIASOURCE_CREATE_FUNC m_pFuncs[MEDIA_TYPE_NUM];
};

#endif
