#ifndef _BITMAPDECODER_H_
#define _BITMAPDECODER_H_

#include "MediaDecoder.h"

class CBitmapDecoder : public CMediaDecoder
{
public:
	CBitmapDecoder();
	~CBitmapDecoder();
	bool SetInputInfo(MediaType type, MediaInfo *pInfo);
	bool GetOutputInfo(MediaInfo *pInfo);
	int GetInputSize(void);
	int GetOutputSize(void);
	int Decode(unsigned char *pInData, int nInSize, void *pOutData);
private:
	int m_nWidth, m_nHeight, m_nBytesPerPixel;
	int m_nCompression;
	unsigned char *m_pColorTable;
};

#endif
