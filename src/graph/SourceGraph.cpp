#include "SourceGraph.h"
#include <string.h>

CSourceGraph::CSourceGraph()
{
	m_pFormatChangedListener = NULL;
	m_pWrapSource = NULL;
	m_pMediaSource = NULL;
	m_pMuxSource = NULL;
}

CSourceGraph::~CSourceGraph()
{
	Close();
}

void CSourceGraph::AddFormatChangedListener(IFormatChangedListener *pListener)
{
	m_pFormatChangedListener = pListener;
}

MediaType CSourceGraph::Open(CDataSource *pDataSource, MediaType type)
{
	Close();

	m_pWrapSource = new CWrapSource(pDataSource, type);
	m_pMediaSource = m_pWrapSource->GetOutputSource(0);  //create a media source
	if (m_pMediaSource == NULL)
	{
		delete m_pWrapSource;
		m_pWrapSource = NULL;
		return MEDIA_TYPE_UNKNOWN;
	}

	BuildGraph(m_pMediaSource);
	if (m_pMuxSource != NULL && m_pFormatChangedListener != NULL)
		m_pFormatChangedListener->FormatChanged(this, m_pMuxSource, 0);
	m_pMediaSource->SeekTime(0);
	return m_pMediaSource->GetSourceType();
}

void CSourceGraph::Close(void)
{
	delete m_pWrapSource;
	m_pWrapSource = NULL;
	delete m_pMuxSource;
	m_pMuxSource = NULL;
}

int CSourceGraph::GetDuration(void)
{
	return m_pMediaSource == NULL? 0 : m_pMediaSource->GetDuration();
}

CMediaSource *CSourceGraph::GetSource(void)
{
	return m_pMediaSource;
}

int CSourceGraph::SeekTime(int nTime)
{
	if (m_pMuxSource != NULL)
		m_pMuxSource->SeekTime(0);
	return m_pMediaSource->SeekTime(nTime);
}

void CSourceGraph::BuildGraph(CMediaSource *pSource)
{
	int i;
	CMediaSource *pTempSource;
	MediaType type;

	for (i = 0; i < pSource->GetOutputNum(); i++)
	{
		type = pSource->GetOutputType(i);
		pTempSource = pSource->GetOutputSource(i);  //create a media source
		if (pTempSource != NULL)
			BuildGraph(pTempSource);
		else if (type == MEDIA_TYPE_MIDIEVENT)
		{
			if (m_pMuxSource == NULL)
				m_pMuxSource = new CMuxSource();
			m_pMuxSource->AddSource(pSource, i);
		}
		else if (m_pFormatChangedListener != NULL)
			m_pFormatChangedListener->FormatChanged(this, pSource, i);
	}
}
