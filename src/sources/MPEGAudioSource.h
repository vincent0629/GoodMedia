#ifndef _MPEGAUDIOSOURCE_H_
#define _MPEGAUDIOSOURCE_H_

#include "MediaSource.h"
#include "MPEGAudioInfo.h"
#include "StreamPos.h"

class CMPEGAudioSource : public CMediaSource
{
public:
	enum
	{
		HEADER_SIZE = 4
	};
public:
	CMPEGAudioSource(CMediaSource *pSource, int nIndex);
	~CMPEGAudioSource();
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

	int NextFrame(bool bSearch);
private:
	MPEGAudioInfo m_MPEGAudioInfo;
	StreamPos m_StreamPos;
	int m_nFrameIndex;
	int m_nSamplesPerFrame;
	int m_nDuration;
};

#endif
