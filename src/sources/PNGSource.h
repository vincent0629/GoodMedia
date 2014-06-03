#ifndef _PNGSOURCE_H_
#define _PNGSOURCE_H_

#include "MediaSource.h"
#include "PNGInfo.h"
#include "StreamPos.h"

class CPNGSource : public CMediaSource
{
public:
	CPNGSource(CMediaSource *pSource, int nIndex);
	~CPNGSource();
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
	PNGInfo m_PNGInfo;
	StreamPos m_Pos;
};

#endif
