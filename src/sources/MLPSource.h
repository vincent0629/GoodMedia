#ifndef _MLPSOURCE_H_
#define _MLPSOURCE_H_

#include "MediaSource.h"
#include "MLPInfo.h"
#include "StreamPos.h"

class CMLPSource : public CMediaSource
{
public:
	enum
	{
		HEADER_SIZE = 12
	};
public:
	CMLPSource(CMediaSource *pSource, int nIndex);
	~CMLPSource();
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
	unsigned char m_pHeader[HEADER_SIZE];

	int NextSync(bool bSearch);
private:
	MLPInfo m_MLPInfo;
	StreamPos m_StreamPos;
};

#endif
