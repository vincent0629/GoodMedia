#ifndef _REALAUDIOSOURCE_H_
#define _REALAUDIOSOURCE_H_

#include "MediaSource.h"
#include "RealMediaInfo.h"

class CRealAudioSource : public CMediaSource
{
public:
	CRealAudioSource(CMediaSource *pSource, int nIndex);
	~CRealAudioSource();
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
	AudioInfo m_AudioInfo;
	RealMediaInfo m_RealMediaInfo;

	char *ReadString(int nSize);
};

#endif
