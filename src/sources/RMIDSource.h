#ifndef _RMIDSOURCE_H_
#define _RMIDSOURCE_H_

#include "MediaSource.h"
#include "StreamPos.h"

class CRMIDSource : public CMediaSource
{
public:
	CRMIDSource(CMediaSource *pSource, int nIndex);
	~CRMIDSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	int GetDuration(void);
	int SeekTime(int nTime);
	size_t GetSize(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
private:
	StreamPos m_StreamPos;
};

#endif
