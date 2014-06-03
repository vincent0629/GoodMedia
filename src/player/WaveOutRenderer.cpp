#include "WaveOutRenderer.h"
#include <string.h>

CWaveOutRenderer::CWaveOutRenderer() : CMediaRenderer()
{
	m_hWaveOut = NULL;
	m_pBuffer = NULL;
	m_nBufferSize = 0;
}

CWaveOutRenderer::~CWaveOutRenderer()
{
	Close();
}

bool CWaveOutRenderer::Open(MediaInfo *pInfo)
{
	AudioRenderInfo *pAudioInfo;
	int i;

	Close();

	if (pInfo->nSize < sizeof(AudioRenderInfo))
		return false;

	pAudioInfo = (AudioRenderInfo *)pInfo;
	InitWaveFormat(pAudioInfo->nSampleRate, pAudioInfo->nBitsPerSample, pAudioInfo->nChannel);

	if (waveOutOpen(&m_hWaveOut, WAVE_MAPPER, (WAVEFORMATEX *)&m_WaveFormat, (DWORD)waveOutProc, (DWORD)this, WAVE_ALLOWSYNC | CALLBACK_FUNCTION) != MMSYSERR_NOERROR)  //open wave out
		return false;

	m_nPlayIndex = 0;
	m_nDataNum = 0;
	m_nBufferSize = pAudioInfo->nSampleRate / 10 * ((pAudioInfo->nBitsPerSample + 7) >> 3) * pAudioInfo->nChannel;  //buffer size is 1/10 second
	m_pBuffer = new char[BUFFER_NUM * m_nBufferSize];
	memset(m_pBuffer, pAudioInfo->nBitsPerSample == 8? 0x80 : 0, BUFFER_NUM * m_nBufferSize);
	for (i = 0; i < BUFFER_NUM; i++)  //initialize wave header
	{
		memset(m_WaveHdr + i, 0, sizeof(WAVEHDR));
		m_WaveHdr[i].dwBufferLength = m_nBufferSize;
		m_WaveHdr[i].lpData = m_pBuffer + i * m_nBufferSize;
		waveOutPrepareHeader(m_hWaveOut, m_WaveHdr + i, sizeof(WAVEHDR));
	}
	m_hEvent = CreateEvent(NULL, false, false, NULL);
	return true;
}

void CWaveOutRenderer::Write(void *pBuffer, int nSize)
{
	char *pcBuffer;
	int i, n, nCount;

	if (m_hWaveOut == NULL)
		return;

	pcBuffer = (char *)pBuffer;
	nCount = nSize;
	if (m_nDataNum == 0)
	{
		n = m_nBufferSize * 2;
		if (n > nCount)
			n = nCount;
		memcpy(m_pBuffer + m_nBufferSize * 2 - n, pcBuffer, n);
		pcBuffer += n;
		nCount -= n;

		m_nPlayIndex = 0;
		m_nWriteIndex = m_nBufferSize * 2;
		ResetEvent(m_hEvent);
		for (i = 0; i < 2; i++)
		{
			waveOutWrite(m_hWaveOut, m_WaveHdr + m_nPlayIndex, sizeof(WAVEHDR));
			++m_nPlayIndex;
			++m_nDataNum;
		}
	}

	while (nCount > 0)
	{
		if (m_nDataNum >= BUFFER_NUM)
			WaitForSingleObject(m_hEvent, INFINITE);

		n = m_nBufferSize - m_nWriteIndex % m_nBufferSize;
		if (n > nCount)
			n = nCount;
		memcpy(m_pBuffer + m_nWriteIndex, pcBuffer, n);
		m_nWriteIndex += n;
		if (m_nWriteIndex >= BUFFER_NUM * m_nBufferSize)
			m_nWriteIndex -= BUFFER_NUM * m_nBufferSize;
		pcBuffer += n;
		nCount -= n;

		if (nCount > 0 || m_nWriteIndex % m_nBufferSize == 0)
		{
			ResetEvent(m_hEvent);
			waveOutWrite(m_hWaveOut, m_WaveHdr + m_nPlayIndex, sizeof(WAVEHDR));
			if (++m_nPlayIndex == BUFFER_NUM)
				m_nPlayIndex = 0;
			++m_nDataNum;
		}
	}

	FireRenderedEvent(pBuffer, nSize);
}

void CWaveOutRenderer::Flush(bool bWait)
{
	char *pBuffer;

	if (!bWait)
		m_nDataNum = 0;
	else if (m_hWaveOut != NULL)
	{
		if (m_nWriteIndex % m_nBufferSize != 0)
		{
			pBuffer = new char[m_nBufferSize];
			memset(pBuffer, m_WaveFormat.Format.wBitsPerSample == 8? 0x80 : 0, m_nBufferSize);
			Write(pBuffer, m_nBufferSize);
			delete[] pBuffer;
		}
		while (m_nDataNum > 0)  //wait for all audio samples are played
			WaitForSingleObject(m_hEvent, INFINITE);
	}
}

void CWaveOutRenderer::Close(void)
{
	int i;

	if (m_hWaveOut == NULL)
		return;

	waveOutReset(m_hWaveOut);
	for (i = 0; i < BUFFER_NUM; i++)
		waveOutUnprepareHeader(m_hWaveOut, m_WaveHdr + i, sizeof(WAVEHDR));
	waveOutClose(m_hWaveOut);
	m_hWaveOut = NULL;

	CloseHandle(m_hEvent);
	m_hEvent = NULL;
	delete[] m_pBuffer;
	m_pBuffer = NULL;
	m_nBufferSize = 0;
}

void CWaveOutRenderer::InitWaveFormat(int nSampleRate, int nBitsPerSample, int nChannel)
{
	InitWaveFormatExt(&m_WaveFormat, nSampleRate, nBitsPerSample, nChannel);
}

void CALLBACK CWaveOutRenderer::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == WOM_DONE)
		((CWaveOutRenderer *)dwInstance)->WaveOutProc();
}

void CWaveOutRenderer::WaveOutProc(void)
{
	if (m_nDataNum > 0)
		--m_nDataNum;
	SetEvent(m_hEvent);
}
