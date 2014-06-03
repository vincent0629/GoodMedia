#ifndef _AACSOURCE_H_
#define _AACSOURCE_H_

#include "MediaSource.h"
#include "AACInfo.h"
#include "StreamPos.h"

class CAACSource : public CMediaSource
{
public:
	enum
	{
		HEADER_SIZE = 6
	};
public:
	CAACSource(CMediaSource *pSource, int nIndex);
	~CAACSource();
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

	int NextFrame(bool bSearch);
private:
	AACInfo m_AACInfo;
	StreamPos m_StreamPos;
	int m_nFrameSize;
};

#endif
