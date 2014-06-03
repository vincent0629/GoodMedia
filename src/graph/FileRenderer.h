#ifndef _FILERENDERER_H_
#define _FILERENDERER_H_

#include "MediaRenderer.h"
#include <stdio.h>

typedef struct _FileRenderInfo : MediaInfo
{
	const char *path;
} FileRenderInfo;

class CFileRenderer : public CMediaRenderer
{
public:
	CFileRenderer();
	~CFileRenderer();
	bool Open(MediaInfo *pInfo);
	void Write(void *pBuffer, int nSize);
	void Flush(bool bWait);
	void Close(void);
private:
	FILE *m_pFile;
	int m_nBufferSize;
};

#endif
