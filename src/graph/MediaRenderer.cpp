#include "MediaRenderer.h"
#include <stdio.h>

CMediaRenderer::CMediaRenderer()
{
	m_pListener = NULL;
}

CMediaRenderer::~CMediaRenderer()
{
}

void CMediaRenderer::SetListener(IMediaRendererListener *pListener)
{
	m_pListener = pListener;
}

void CMediaRenderer::FireRenderedEvent(void *pData, int nSize)
{
	if (m_pListener != NULL)
		m_pListener->MediaRendered(this, pData, nSize);
}
