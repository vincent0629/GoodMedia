#ifndef _WAVSOURCE_H_
#define _WAVSOURCE_H_

#include "MediaSource.h"
#include "WAVInfo.h"
#include "StreamPos.h"

class CWAVSource : public CMediaSource
{
public:
	CWAVSource(CMediaSource *pSource, int nIndex);
	~CWAVSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetSize(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetOutputTime(int nIndex);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	WAVInfo m_WAVInfo;
	StreamPos m_StreamPos;
	int m_nDuration;
};

#endif
