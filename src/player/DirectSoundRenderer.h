#ifndef _DIRECTSOUNDRENDERER_H_
#define _DIRECTSOUNDRENDERER_H_

#include "MediaRenderer.h"
#include "WaveFormatExtensible.h"
#include <windows.h>
#include <dsound.h>

class CDirectSoundRenderer : public CMediaRenderer
{
public:
	CDirectSoundRenderer();
	~CDirectSoundRenderer();
	bool Open(MediaInfo *pInfo);
	void Write(void *pBuffer, int nSize);
	void Flush(bool bWait);
	void Close(void);
private:
	WAVEFORMATEXTENSIBLE m_WaveFormat;
	int m_nBufferSize, m_nBytesInBuffer;
	int m_nBufferIndex, m_nInputBufferNum, m_nOutputBufferNum;
	LPDIRECTSOUND m_pDS;
	LPDIRECTSOUNDBUFFER m_pPriBuf;
	LPDIRECTSOUNDBUFFER m_pSecBuf;
	HANDLE m_phNotifyEvents[3], m_hEvent;
	char *m_pBuffer;
	bool m_bPlaying;
	int m_nTimer;

	static void CALLBACK TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
	void TimerCallback(void);
};

#endif
