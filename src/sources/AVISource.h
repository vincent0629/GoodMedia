#ifndef _AVISOURCE_H_
#define _AVISOURCE_H_

#include "MediaSource.h"
#include "StreamPos.h"
#include "AVIInfo.h"
#include "BitmapInfo.h"
#include "WAVInfo.h"

class CAVISource : public CMediaSource
{
public:
	typedef enum
	{
		STREAM_AUDIO,
		STREAM_VIDEO
	} StreamType;
public:
	CAVISource(CMediaSource *pSource, int nIndex);
	~CAVISource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int GetOutputTime(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
protected:
	int NextChunk(char *chunkID);
private:
	AVIInfo m_AVIInfo;
	BitmapInfo m_BitmapInfo;
	WAVInfo m_WAVInfo;
	MediaType m_pTypes[10];
	StreamType m_pStreamTypes[10];
	size_t m_nFileSize;
	int m_nMicrosecondsPerFrame;
	int m_nFrameNum, m_nFrameIndex;
	StreamPos m_pStreamPos[10];
	int m_nBytesPerSecond;
	unsigned char *m_pColorTable;
	int m_nDuration;
};

#endif
