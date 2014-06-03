#ifndef _MIDIOUTRENDERER_H_
#define _MIDIOUTRENDERER_H_

#include "MediaRenderer.h"
#include <windows.h>
#include <mmsystem.h>

class CMIDIOutRenderer : public CMediaRenderer
{
public:
	CMIDIOutRenderer();
	~CMIDIOutRenderer();
	bool Open(MediaInfo *pInfo);
	void Write(void *pBuffer, int nSize);
	void Flush(bool bWait);
	void Close(void);
private:
	HMIDIOUT m_hmo;
	char m_pLyrics[0x100];
};

#endif
