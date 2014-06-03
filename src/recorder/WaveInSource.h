#ifndef _WAVEINSOURCE_H_
#define _WAVEINSOURCE_H_

#include "DataSource.h"
#include "platform.h"
#include <windows.h>
#include <mmsystem.h>

class CWaveInSource : public CDataSource
{
public:
	enum
	{
		BUFFER_NUM = 3
	};
public:
	CWaveInSource();
	~CWaveInSource();
	bool Open(const TCHAR *path);
	bool Open(int nSampleRate, int nBitsPerSample, int nChannel);
	int ReadData(void *pBuffer, int nSize);
	void Close(void);
private:
	WAVEFORMATEX m_WaveFormat;
	HWAVEIN m_hWaveIn;
	WAVEHDR m_WaveHdr[BUFFER_NUM];
	int m_nRecordIndex, m_nReadIndex, m_nDataNum;
	char *m_pBuffer;
	int m_nBufferSize;
	HANDLE m_hEvent;
	bool m_bStarted;

	static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	void WaveInProc(void);
};

#endif
