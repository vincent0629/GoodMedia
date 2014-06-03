#ifndef _THEORASOURCE_H_
#define _THEORASOURCE_H_

#include "MediaSource.h"
#include "TheoraInfo.h"

class CTheoraSource : public CMediaSource
{
public:
	CTheoraSource(CMediaSource *pSource, int nIndex);
	~CTheoraSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	TheoraInfo m_Info;
};

#endif
