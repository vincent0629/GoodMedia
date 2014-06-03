#include "RendererFactory.h"
#include "FileRenderer.h"
#include <stdio.h>

RENDERER_CREATE_FUNC CRendererFactory::m_pFuncs[RENDER_TYPE_NUM] = {NULL};

void CRendererFactory::Register(RenderType type, RENDERER_CREATE_FUNC func)
{
	m_pFuncs[type] = func;
}

CMediaRenderer *CRendererFactory::Create(RenderType type)
{
	if (type == RENDER_TYPE_FILE)
		return new CFileRenderer();

	if (m_pFuncs[type] == NULL)
		return NULL;
	return (*m_pFuncs[type])(type);
}
