#ifndef _AIFFSOURCE_H_
#define _AIFFSOURCE_H_

#include "MediaSource.h"
#include "AIFFInfo.h"
#include "PCMInfo.h"
#include "StreamPos.h"

class CAIFFSource : public CMediaSource
{
public:
	CAIFFSource(CMediaSource *pSource, int nIndex);
	~CAIFFSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int SeekTime(int nTime);
	int GetOutputTime(int nIndex);
	int GetDuration(void);
private:
	bool m_bCompressed;
	AIFFInfo m_AIFFInfo;
	PCMInfo m_PCMInfo;
	MediaType m_OutputType;
	int m_nBytesPerSample;
	int m_nDuration;
	StreamPos m_StreamPos;
};

#endif
