#ifndef _MIDISOURCE_H_
#define _MIDISOURCE_H_

#include "MediaSource.h"
#include "MIDIInfo.h"
#include "StreamPos.h"

class CMIDISource : public CMediaSource
{
public:
	typedef struct
	{
		int nTicks;
		int nStatus;
	} MIDITrack;
	typedef struct
	{
		int nTempo;
		int nTicks;
		int nTime;
	} TempoEvent;
public:
	CMIDISource(CMediaSource *pSource, int nIndex);
	~CMIDISource();
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
protected:
	int ReadVarLen(void);
	int TicksToTime(int nTick);
private:
	MIDIInfo m_MIDIInfo;
	int m_nTicksPerQuarterNote;
	StreamPos *m_pStreamPos;
	MIDITrack *m_pTracks;
	TempoEvent *m_pTempoEvents;
	int m_nTempoEventNum;
	int m_nDuration;
	int m_nSeekTime;

};

#endif
