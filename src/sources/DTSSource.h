#ifndef _DTSSOURCE_H_
#define _DTSSOURCE_H_

#include "MediaSource.h"
#include "DTSInfo.h"
#include "StreamPos.h"

class CDTSSource : public CMediaSource
{
public:
	enum
	{
		HEADER_SIZE = 15
	};
public:
	CDTSSource(CMediaSource *pSource, int nIndex);
	~CDTSSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
protected:
	unsigned char m_pHeader[HEADER_SIZE];

	int NextSync(bool bSearch);
private:
	DTSInfo m_DTSInfo;
	StreamPos m_StreamPos;
	int m_nFrameSize;
};

#endif
