#ifndef _MPEGPROGRAMSOURCE_H_
#define _MPEGPROGRAMSOURCE_H_

#include "MediaSource.h"
#include "MPEGProgramInfo.h"
#include "StreamPos.h"

class CMPEGProgramSource : public CMediaSource
{
public:
	CMPEGProgramSource(CMediaSource *pSource, int nIndex);
	~CMPEGProgramSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
	int GetOutputTime(int nIndex);
	int NextPacket(int *pnPacketLen = NULL);
protected:
	int NextSync(void);
private:
	MPEGProgramInfo m_MPEGProgramInfo;
	int m_nDuration;
	StreamPos *m_pStreamPos;
	int *m_pnCodes;

	size_t ParsePTS(int nStreamID);
};

#endif
