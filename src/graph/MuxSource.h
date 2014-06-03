#ifndef _MUXSOURCE_H_
#define _MUXSOURCE_H_

#include "MediaSource.h"

class CMuxSource : public CMediaSource
{
public:
	enum
	{
		MAX_SOURCE_NUM = 50
	};
	typedef struct
	{
		CMediaSource *pSource;
		int nIndex;
		unsigned char *pBuffer;
		int nBufferBytes, nBufferSize;
		int nTime;
	} SourceParam;
public:
	CMuxSource();
	~CMuxSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	int GetOutputTime(int nIndex);
	int GetDuration(void);
	int SeekTime(int nTime);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	void AddSource(CMediaSource *pSource, int nIndex);
private:
	int m_nSourceNum;
	SourceParam m_pSourceParams[MAX_SOURCE_NUM];
	MediaType m_MediaType;
	int m_nDuration, m_nOutputTime;
};

#endif
