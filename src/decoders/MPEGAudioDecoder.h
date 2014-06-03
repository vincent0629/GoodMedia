#ifndef _MPEGAUDIODECODER_H_
#define _MPEGAUDIODECODER_H_

#include "MediaDecoder.h"

class CMPEGAudioDecoder : public CMediaDecoder
{
public:
	typedef struct
	{
		unsigned char *pInData;
		int nInSize;
		short *V[2];
	} DECODERPARAM;
public:
	CMPEGAudioDecoder();
	~CMPEGAudioDecoder();
	bool SetInputInfo(MediaType type, MediaInfo *pInfo);
	bool GetOutputInfo(MediaInfo *pInfo);
	int GetInputSize(void);
	int GetOutputSize(void);
	int Decode(unsigned char *pInData, int nInSize, void *pOutData);
	void Flush(void);
private:
	int m_nSampleRate;
	int m_nChannel;
	DECODERPARAM m_Param;

	static int ReadByte(void *pData);
};

#endif
