#ifndef _DATSOURCE_H_
#define _DATSOURCE_H_

#include "MediaSource.h"
#include "StreamPos.h"

class CDATSource : public CMediaSource
{
public:
	CDATSource(CMediaSource *pSource, int nIndex);
	~CDATSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	size_t GetSize(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	int m_nDuration;
	size_t m_nSize;
	StreamPos m_StreamPos;
};

#endif
