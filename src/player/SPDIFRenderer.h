#ifndef _SPDIFRENDERER_H_
#define _SPDIFRENDERER_H_

#include "WaveOutRenderer.h"

class CSPDIFRenderer : public CWaveOutRenderer
{
protected:
	void InitWaveFormat(int nSampleRate, int nBitsPerSample, int nChannel);
};

#endif
