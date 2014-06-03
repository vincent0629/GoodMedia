#ifndef _MEDIASOURCE_H_
#define _MEDIASOURCE_H_

#include "MediaInfo.h"
#include <stdio.h>

class CMediaSource
{
public:
	virtual ~CMediaSource();
	//Get media type of this media source
	virtual MediaType GetSourceType(void) = 0;
	//Get info of this media source
	virtual bool GetSourceInfo(MediaInfo *pInfo) = 0;
	//Get number of output
	virtual int GetOutputNum(void);
	//Get media type of specified output
	virtual MediaType GetOutputType(int nIndex) = 0;
	//Get info of specified output, default is false, this function should be overrided
	virtual bool GetOutputInfo(int nIndex, MediaInfo *pInfo);
	//Get a media source with media type of specified outpuT
	virtual CMediaSource *GetOutputSource(int nIndex);
	//Get current available size of specified output, default is 0, this function should be overrided
	virtual size_t GetSize(int nIndex);
	//Get current position from specified output
	virtual size_t GetPosition(int nIndex) = 0;
	//Read a byte from specified output
	//return the byte, -1 means end of data source, this function may be overrided if needed
	virtual int ReadData(int nIndex);
	//Read specified bytes from specified output into a buffer
	//return number of bytes read actually
	virtual int ReadData(int nIndex, void *pData, int nSize) = 0;
	//Read specified bytes from specified output and convert to an integer
	//return data
	int ReadLittleEndian(int nIndex, int nSize);
	//Read specified bytes reversely from specified output and convert to an integer
	//return data
	int ReadBigEndian(int nIndex, int nSize);
	//Seek specified output to specified position
	//nFrom is SEEK_SET, SEEK_CUR, or SEEK_END
	//return success or not
	virtual bool SeekData(int nIndex, size_t nOffset, int nFrom) = 0;
	//Get current time stamp of specified output, default is 0, this function should be overrided
	virtual int GetOutputTime(int nIndex);
	//Get duration
	virtual int GetDuration(void) = 0;
	virtual int SeekTime(int nTime) = 0;
	CMediaSource *GetParent(void);
	int GetIndex(void);
protected:
	CMediaSource *m_pSource;
	int m_nSourceIndex;
	int m_nOutputNum;
	CMediaSource **m_ppOutputSources;
	bool *m_pGotOutputSources;

	CMediaSource();
	CMediaSource(CMediaSource *pSource, int nIndex);
	MediaType DetectOutputType(int nMode, int nIndex);
	int SourceGetOutputTime(int nIndex);
	size_t SourceGetSize(void);
	size_t SourceGetPosition(void);
	int SourceReadData(void);
	int SourceReadData(void *pData, int nSize);
	int SourceReadLittleEndian(int nSize);
	int SourceReadBigEndian(int nSize);
	bool SourceSeekData(size_t nOffset, int nFrom);
	static MediaType AudioType(long nFormatTag);
	static MediaType VideoType(long nCompression);
};

#endif
