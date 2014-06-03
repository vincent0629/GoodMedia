#ifndef _DECODERFACTORY_H_
#define _DECODERFACTORY_H_

#include "MediaDecoder.h"

typedef CMediaDecoder *(*DECODER_CREATE_FUNC)(MediaType type);

class CDecoderFactory
{
public:
	static void Register(MediaType type, DECODER_CREATE_FUNC func);
	static CMediaDecoder *Create(MediaType type);
private:
	static DECODER_CREATE_FUNC m_pFuncs[MEDIA_TYPE_NUM];
};

#endif
