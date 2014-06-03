#ifndef _PCMDECODER_H_
#define _PCMDECODER_H_

#include "MediaDecoder.h"
#include "PCMInfo.h"

class CPCMDecoder : public CMediaDecoder
{
public:
	CPCMDecoder();
	~CPCMDecoder();
	bool SetInputInfo(MediaType type, MediaInfo *pInfo);
	bool GetOutputInfo(MediaInfo *pInfo);
	int GetInputSize(void);
	int GetOutputSize(void);
	int Decode(unsigned char *pInData, int nInSize, void *pOutData);
private:
	PCMInfo m_Info;
	int m_nBpsIn, m_nBpsOut;
	bool m_bSwapOrder;
};

#endif
