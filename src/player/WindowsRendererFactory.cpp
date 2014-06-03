#include "WindowsRendererFactory.h"
#include "RendererFactory.h"
#include "WaveOutRenderer.h"
#include "DIBRenderer.h"
#include "SPDIFRenderer.h"
#include "MIDIOutRenderer.h"
#include "TextRenderer.h"
#include <stdio.h>

static CMediaRenderer *createFunc(RenderType type)
{
	CMediaRenderer *pRenderer;

	pRenderer = NULL;
	switch (type)
	{
		case RENDER_TYPE_AUDIO:
			pRenderer = new CWaveOutRenderer();
			break;
		case RENDER_TYPE_VIDEO:
			pRenderer = new CDIBRenderer();
			break;
		case RENDER_TYPE_SPDIF:
			pRenderer = new CSPDIFRenderer();
			break;
		case RENDER_TYPE_MIDI:
			pRenderer = new CMIDIOutRenderer();
			break;
		case RENDER_TYPE_TEXT:
			pRenderer = new CTextRenderer();
			break;
		default:
			break;
	}
	return pRenderer;
}

void CWindowsRendererFactory::Register(void)
{
	CRendererFactory::Register(RENDER_TYPE_AUDIO, createFunc);
	CRendererFactory::Register(RENDER_TYPE_VIDEO, createFunc);
	CRendererFactory::Register(RENDER_TYPE_SPDIF, createFunc);
	CRendererFactory::Register(RENDER_TYPE_MIDI, createFunc);
	CRendererFactory::Register(RENDER_TYPE_TEXT, createFunc);
}
