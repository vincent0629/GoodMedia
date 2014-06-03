#ifndef _RENDERERFACTORY_H_
#define _RENDERERFACTORY_H_

#include "MediaRenderer.h"

typedef CMediaRenderer *(*RENDERER_CREATE_FUNC)(RenderType type);

class CRendererFactory
{
public:
	static void Register(RenderType type, RENDERER_CREATE_FUNC func);
	static CMediaRenderer *Create(RenderType type);
private:
	static RENDERER_CREATE_FUNC m_pFuncs[RENDER_TYPE_NUM];
};

#endif
