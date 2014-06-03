#ifndef _PESPACKETSOURCE_H_
#define _PESPACKETSOURCE_H_

#include "MediaSource.h"
#include "PESPacketInfo.h"
#include "PCMInfo.h"
#include "StreamPos.h"

class CPESPacketSource : public CMediaSource
{
public:
	CPESPacketSource(CMediaSource *pSource, int nIndex);
	~CPESPacketSource();
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
private:
	PESPacketInfo m_Info;
	StreamPos m_StreamPos;
	MediaType m_OutputType;
	PCMInfo m_PCMInfo;
	int m_nMPEGVersion;

	int NextPacket(size_t *pPTS);
};

#endif
