#ifndef _BitmapSOURCE_H_
#define _BitmapSOURCE_H_

#include "MediaSource.h"
#include "BitmapInfo.h"
#include "StreamPos.h"

class CBitmapSource : public CMediaSource
{
public:
	CBitmapSource(CMediaSource *pSource, int nIndex);
	~CBitmapSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetSize(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	BitmapInfo m_BitmapInfo;
	StreamPos m_StreamPos;
};

#endif
