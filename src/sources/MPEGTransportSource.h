#ifndef _MPEGTRANSPORTSOURCE_H_
#define _MPEGTRANSPORTSOURCE_H_

#include "MediaSource.h"
#include "ProgramAssociationSource.h"
#include "ProgramMapSource.h"
#include "StreamPos.h"

class CMPEGTransportSource : public CMediaSource
{
public:
	CMPEGTransportSource(CMediaSource *pSource, int nIndex);
	~CMPEGTransportSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	CMediaSource *GetOutputSource(int nIndex);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
	int NextPacket(int *pnLen = NULL);
protected:
	int NextSync(bool bSearch);
private:
	size_t m_nFirstSyncPos;
	int m_nPacketSize;
	CProgramAssociationSource *m_pPASource;
	ProgramAssociationInfo m_PAInfo;
	CProgramMapSource **m_ppPMSource;
	ProgramMapInfo *m_pPMInfo;
	StreamPos m_pStreamPos[100];

	StreamInfo *GetStreamInfo(int nIndex);
};

#endif
