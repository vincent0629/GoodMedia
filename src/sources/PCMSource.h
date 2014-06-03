#ifndef _PCMSOURCE_H_
#define _PCMSOURCE_H_

#include "MediaSource.h"
#include "PCMInfo.h"
#include "StreamPos.h"

class CPCMSource : public CMediaSource
{
public:
	CPCMSource(CMediaSource *pSource, int nIndex);
	~CPCMSource();
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
	MediaType m_Type;
	PCMInfo m_Info;
	StreamPos m_StreamPos;
	int m_nSampleRate;
	int m_nBytesPerSample;
};

#endif
