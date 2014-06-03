#ifndef _H264SOURCE_H_
#define _H264SOURCE_H_

#include "MediaSource.h"

class CH264Source : public CMediaSource
{
public:
	CH264Source(CMediaSource *pSource, int nIndex);
	~CH264Source();
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
	int NextSync(void);
private:
	VideoInfo m_H264Info;
};

#endif
