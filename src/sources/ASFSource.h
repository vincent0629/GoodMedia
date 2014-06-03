#ifndef _ASFSOURCE_H_
#define _ASFSOURCE_H_

#include "MediaSource.h"
#include "ASFInfo.h"
#include "StreamPos.h"

class CASFSource : public CMediaSource
{
public:
	typedef struct
	{
		MediaType type;
		int nStreamNumber;
		long nFormat;
		MediaInfo *pInfo;
		int nPropertyFlags;
		int nPadding;
		bool bMultiplePayloads;
		int nPayloadFlags;
		int nNumOfPayloads;
	} StreamData;
public:
	CASFSource(CMediaSource *pSource, int nIndex);
	~CASFSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	//CMediaSource *GetOutputSource(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	ASFInfo m_ASFInfo;
	int m_nDuration;
	StreamPos *m_pStreamPos;
	StreamData m_pStreamData[30];
	int m_nPacketIndex, m_nPacketNum, m_nPacketSize;
};

#endif
