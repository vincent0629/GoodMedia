#ifndef _VORBISSOURCE_H_
#define _VORBISSOURCE_H_

#include "MediaSource.h"
#include "VorbisInfo.h"

class CVorbisSource : public CMediaSource
{
public:
	CVorbisSource(CMediaSource *pSource, int nIndex);
	~CVorbisSource();
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
	VorbisInfo m_VorbisInfo;
};

#endif
