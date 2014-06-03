#ifndef _FLASHVIDEOSOURCE_H_
#define _FLASHVIDEOSOURCE_H_

#include "MediaSource.h"
#include "FlashVideoInfo.h"
#include "PCMInfo.h"
#include "StreamPos.h"

class CFlashVideoSource : public CMediaSource
{
public:
	CFlashVideoSource(CMediaSource *pSource, int nIndex);
	~CFlashVideoSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	FlashVideoInfo m_Info;
	StreamPos m_StreamPos[2];
	MediaType m_AudioType, m_VideoType;
	PCMInfo m_PCMInfo;

	int NextTag(int *pnLength);
};

#endif
