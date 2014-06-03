#ifndef _AC3SOURCE_H_
#define _AC3SOURCE_H_

#include "MediaSource.h"
#include "StreamPos.h"

class CAC3Source : public CMediaSource
{
public:
	enum
	{
		HEADER_SIZE = 8
	};
public:
	CAC3Source(CMediaSource *pSource, int nIndex);
	~CAC3Source();
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
protected:
	unsigned char m_pHeader[HEADER_SIZE];

	int NextSync(bool bSearch);
private:
	MediaType m_SourceType;
	AudioInfo m_AC3Info;
	bool m_bSwapByte;
	StreamPos m_StreamPos;
	int m_nFrameSize, m_nFrameIndex;
};

#endif
