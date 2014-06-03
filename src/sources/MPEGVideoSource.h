#ifndef _MPEGVIDEOSOURCE_H_
#define _MPEGVIDEOSOURCE_H_

#include "MediaSource.h"
#include "MPEGVideoInfo.h"

class CMPEGVideoSource : public CMediaSource
{
public:
	CMPEGVideoSource(CMediaSource *pSource, int nIndex);
	~CMPEGVideoSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
	int GetOutputTime(int nIndex);
protected:
	int NextSync(void);
private:
	MPEGVideoInfo m_MPEGVideoInfo;
	unsigned long m_nBuffer;
};

#endif
