#ifndef _MATROSKASOURCE_H_
#define _MATROSKASOURCE_H_

#include "MediaSource.h"
#include "MatroskaInfo.h"

class CMatroskaSource : public CMediaSource
{
public:
	CMatroskaSource(CMediaSource *pSource, int nIndex);
	~CMatroskaSource();
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
	unsigned int ReadID(void);
	unsigned int ReadUint(void);
private:
	MatroskaInfo m_MatroskaInfo;
	int m_nDuration;
};

#endif
