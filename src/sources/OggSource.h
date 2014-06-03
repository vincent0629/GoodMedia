#ifndef _OGGSOURCE_H_
#define _OGGSOURCE_H_

#include "MediaSource.h"
#include "StreamPos.h"

class COggSource : public CMediaSource
{
public:
	COggSource(CMediaSource *pSource, int nIndex);
	~COggSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
protected:
	int NextPage(unsigned long *pnSerial);
private:
	MediaType m_OutputType[2];
	StreamPos m_StreamPos[2];
	int m_nHeaderType;
	unsigned long m_nSerials[2];
};

#endif
