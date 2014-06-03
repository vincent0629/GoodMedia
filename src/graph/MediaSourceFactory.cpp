#include "MediaSourceFactory.h"

MEDIASOURCE_CREATE_FUNC CMediaSourceFactory::m_pFuncs[MEDIA_TYPE_NUM] = {NULL};

void CMediaSourceFactory::Register(MediaType type, MEDIASOURCE_CREATE_FUNC func)
{
	m_pFuncs[type] = func;
}

CMediaSource *CMediaSourceFactory::Create(MediaType type, CMediaSource *pSource, int nIndex)
{
	if (m_pFuncs[type] == NULL)
		return NULL;
	return (*m_pFuncs[type])(type, pSource, nIndex);
}
