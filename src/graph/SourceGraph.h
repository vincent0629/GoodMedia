#ifndef _SOURCEGRAPH_H_
#define _SOURCEGRAPH_H_

#include "WrapSource.h"
#include "MuxSource.h"
#include "FormatChangedListener.h"

class CSourceGraph
{
public:
	CSourceGraph();
	~CSourceGraph();
	void AddFormatChangedListener(IFormatChangedListener *pListener);
	MediaType Open(CDataSource *pDataSource, MediaType type = MEDIA_TYPE_UNKNOWN);
	void Close(void);
	int GetDuration(void);
	CMediaSource *GetSource(void);
	int SeekTime(int nTime);
protected:
	void BuildGraph(CMediaSource *pSource);
private:
	IFormatChangedListener *m_pFormatChangedListener;
	CWrapSource *m_pWrapSource;
	CMediaSource *m_pMediaSource;
	CMuxSource *m_pMuxSource;
};

#endif
