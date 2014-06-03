#ifndef _WAVEOUTRENDERER_H_
#define _WAVEOUTRENDERER_H_

#include "MediaRenderer.h"
#include "WaveFormatExtensible.h"

class CWaveOutRenderer : public CMediaRenderer
{
public:
	enum
	{
		BUFFER_NUM = 3
	};
public:
	CWaveOutRenderer();
	~CWaveOutRenderer();
	bool Open(MediaInfo *pInfo);
	void Write(void *pBuffer, int nSize);
	void Flush(bool bWait);
	void Close(void);
protected:
	WAVEFORMATEXTENSIBLE m_WaveFormat;

	virtual void InitWaveFormat(int nSampleRate, int nBitsPerSample, int nChannel);
private:
	HWAVEOUT m_hWaveOut;
	WAVEHDR m_WaveHdr[BUFFER_NUM];
	int m_nPlayIndex, m_nWriteIndex, m_nDataNum;
	char *m_pBuffer;
	int m_nBufferSize;
	HANDLE m_hEvent;

	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	void WaveOutProc(void);
};

#endif
