#ifndef _PROGRAMMAPSOURCE_H_
#define _PROGRAMMAPSOURCE_H_

#include "MediaSource.h"
#include "ProgramMapInfo.h"

class CProgramMapSource : public CMediaSource
{
public:
	CProgramMapSource(CMediaSource *pSource, int nIndex);
	~CProgramMapSource();
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
	ProgramMapInfo m_Info;
};

#endif
