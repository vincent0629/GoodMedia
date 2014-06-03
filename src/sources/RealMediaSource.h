#ifndef _REALSOURCE_H_
#define _REALSOURCE_H_

#include "MediaSource.h"
#include "RealMediaInfo.h"

class CRealMediaSource : public CMediaSource
{
public:
	CRealMediaSource(CMediaSource *pSource, int nIndex);
	~CRealMediaSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	int GetDuration(void);
	int SeekTime(int nTime);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
protected:
	void ReadString(char *str, int nSize);
	char *ReadString(int nSize);
private:
	RealMediaInfo m_RealMediaInfo;
	MediaType m_pOutputTypes[5];
	int m_nDuration;
};

#endif
