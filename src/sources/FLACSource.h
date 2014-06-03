#ifndef _FLACSOURCE_H_
#define _FLACSOURCE_H_

#include "MediaSource.h"
#include "FLACInfo.h"

class CFLACSource : public CMediaSource
{
public:
	CFLACSource(CMediaSource *pSource, int nIndex);
	~CFLACSource();
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
	FLACInfo m_FLACInfo;
	int m_nDuration;
};

#endif
