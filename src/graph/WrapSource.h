#ifndef _WRAPSOURCE_H_
#define _WRAPSOURCE_H_

#include "DataSource.h"
#include "MediaSource.h"

class CWrapSource : public CMediaSource
{
public:
	CWrapSource(CDataSource *pSource, MediaType type = MEDIA_TYPE_UNKNOWN);
	~CWrapSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	int GetDuration(void);
	int SeekTime(int nTime);
	size_t GetSize(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	int ReadBigEndian(int nIndex, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
private:
	CDataSource *m_pSource;
	MediaType m_OutputType;
};

#endif
