#ifndef _MEDIADECODER_H_
#define _MEDIADECODER_H_

#include "MediaInfo.h"

class CMediaDecoder
{
public:
	CMediaDecoder();
	virtual ~CMediaDecoder();
	virtual bool SetInputInfo(MediaType type, MediaInfo *pInfo) = 0;
	virtual bool GetOutputInfo(MediaInfo *pInfo) = 0;
	virtual int GetInputSize(void) = 0;
	virtual int GetOutputSize(void) = 0;
	virtual int Decode(unsigned char *pInData, int nInSize, void *pOutData) = 0;
	virtual int SPDIF(unsigned char *pInData, int nInSize, unsigned char *pOutData);
	virtual void Flush(void);
};

#endif
