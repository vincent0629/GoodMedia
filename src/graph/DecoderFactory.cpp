#include "DecoderFactory.h"
#include <stdio.h>

DECODER_CREATE_FUNC CDecoderFactory::m_pFuncs[MEDIA_TYPE_NUM] = {NULL};

void CDecoderFactory::Register(MediaType type, DECODER_CREATE_FUNC func)
{
	m_pFuncs[type] = func;
}

CMediaDecoder *CDecoderFactory::Create(MediaType type)
{
	if (m_pFuncs[type] == NULL)
		return NULL;
	return (*m_pFuncs[type])(type);
}
