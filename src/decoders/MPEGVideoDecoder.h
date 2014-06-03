#ifndef _MPEGVIDEODECODER_H_
#define _MPEGVIDEODECODER_H_

#include "MediaDecoder.h"
#include "huffman.h"
#include "dataqueue.h"

class CMPEGVideoDecoder : public CMediaDecoder
{
public:
	typedef struct
	{
		int nWidth, nHeight;
		unsigned char *pBuffer;
		HUFFMANNODE *pMbAddrIncTree, *pMbTypeTree[4], *pDCSizeLuminanceTree, *pDCSizeChrominanceTree, *pDCTCoeffTree;
		unsigned char pIntraQuantizerMatrix[64], pNonIntraQuantizerMatrix[64];
		double pCos[8][8];
	} DECODERPARAM;
public:
	CMPEGVideoDecoder();
	~CMPEGVideoDecoder();
	bool SetInputInfo(MediaType type, MediaInfo *pInfo);
	bool GetOutputInfo(MediaInfo *pInfo);
	int GetInputSize(void);
	int GetOutputSize(void);
	int Decode(unsigned char *pInData, int nInSize, void *pOutData);
	void Flush(void);
private:
	DECODERPARAM m_Param;
	float m_fPelAspectRatio;
	DATAQUEUE *m_pQueue;

	static int ReadByte(void *pData);
};

#endif
