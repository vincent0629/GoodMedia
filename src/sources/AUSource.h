#ifndef _AUSOURCE_H_
#define _AUSOURCE_H_

#include "MediaSource.h"
#include "PCMInfo.h"
#include "StreamPos.h"

class CAUSource : public CMediaSource
{
public:
	CAUSource(CMediaSource *pSource, int nIndex);
	~CAUSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetOutputTime(int nIndex);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	PCMInfo m_Info;
	StreamPos m_Pos;
	int m_nDuration;
	int m_nBytesPerSample;
};

#endif
