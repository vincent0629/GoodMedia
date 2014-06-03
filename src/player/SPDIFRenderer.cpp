#include "SPDIFRenderer.h"

void CSPDIFRenderer::InitWaveFormat(int nSampleRate, int nBitsPerSample, int nChannel)
{
	CWaveOutRenderer::InitWaveFormat(48000, 16, 2);
	m_WaveFormat.Format.wFormatTag = 0x92;
}
