#ifndef _PROGRAMASSOCIATIONSOURCE_H_
#define _PROGRAMASSOCIATIONSOURCE_H_

#include "MediaSource.h"
#include "ProgramAssociationInfo.h"

class CProgramAssociationSource : public CMediaSource
{
public:
	CProgramAssociationSource(CMediaSource *pSource, int nIndex);
	~CProgramAssociationSource();
	MediaType GetSourceType(void);
	bool GetSourceInfo(MediaInfo *pInfo);
	MediaType GetOutputType(int nIndex);
	bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	size_t GetPosition(int nIndex);
	int ReadData(int nIndex, void *pData, int nSize);
	bool SeekData(int nIndex, size_t nOffset, int nFrom);
	int GetDuration(void);
	int SeekTime(int nTime);
private:
	ProgramAssociationInfo m_Info;
};

#endif
