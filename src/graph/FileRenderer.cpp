#include "FileRenderer.h"
#include <string.h>

CFileRenderer::CFileRenderer() : CMediaRenderer()
{
	m_pFile = NULL;
}

CFileRenderer::~CFileRenderer()
{
	Close();
}

bool CFileRenderer::Open(MediaInfo *pInfo)
{
	FileRenderInfo *pRenderInfo;

	Close();

	if (pInfo->nSize < sizeof(FileRenderInfo))
		return false;

	pRenderInfo = (FileRenderInfo *)pInfo;
	m_pFile = fopen(pRenderInfo->path, "wb");
	return m_pFile != NULL;
}

void CFileRenderer::Write(void *pBuffer, int nSize)
{
	if (m_pFile != NULL)
	{
		fwrite(pBuffer, 1, nSize, m_pFile);
		FireRenderedEvent(pBuffer, nSize);
	}
}

void CFileRenderer::Flush(bool bWait)
{
	if (m_pFile != NULL)
		fflush(m_pFile);
}

void CFileRenderer::Close(void)
{
	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}
