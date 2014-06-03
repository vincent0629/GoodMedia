#ifndef _QUICKTIMESOURCE_H_
#define _QUICKTIMESOURCE_H_

#include "MediaSource.h"
#include "QuickTimeInfo.h"
#include "StreamPos.h"
#include <stdlib.h>

class CQuickTimeSource : public CMediaSource
{
public:
	typedef struct
	{
		int nIndex;
		int nOffset;
	} ChunkStruct;
public:
	CQuickTimeSource(CMediaSource *pSource, int nIndex);
	~CQuickTimeSource();
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
	int64_t NextAtom(char *pType);
private:
	QuickTimeInfo m_Info;
	MediaType m_SourceType;
	int m_nDuration;
	ChunkStruct *m_pChunkTable;
	int m_nChunkTableSize;
	StreamPos m_pStreamPos[10];
	int m_pnChunkIndex[10];
	MediaType m_pTypes[10];
	char m_pFormats[10][5];

	MediaType MediaTypeOf(char *str);
};

#endif
