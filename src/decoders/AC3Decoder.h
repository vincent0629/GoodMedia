#ifndef _AC3DECODER_H_
#define _AC3DECODER_H_

#include "MediaDecoder.h"
#include "MediaInfo.h"

class CAC3Decoder : public CMediaDecoder
{
public:
	CAC3Decoder();
	~CAC3Decoder();
	bool SetInputInfo(MediaType type, MediaInfo *pInfo);
	bool GetOutputInfo(MediaInfo *pInfo);
	int GetInputSize(void);
	int GetOutputSize(void);
	int Decode(unsigned char *pInData, int nInSize, void *pOutData);
	int SPDIF(unsigned char *pInData, int nInSize, unsigned char *pOutData);
private:
	AudioInfo m_Info;
};

#endif
