#ifndef _MONKEYSAUDIOSOURCE_H_
#define _MONKEYSAUDIOSOURCE_H_

#include "MediaSource.h"

class CMonkeysAudioSource : public CMediaSource
{
public:
	CMonkeysAudioSource(CMediaSource *pSource, int nIndex);
	~CMonkeysAudioSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
};

#endif
