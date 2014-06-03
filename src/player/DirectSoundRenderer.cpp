#include "DirectSoundRenderer.h"
#include <stdio.h>

CDirectSoundRenderer::CDirectSoundRenderer() : CMediaRenderer()
{
	HWND hwnd;
	DSBUFFERDESC desc;
	int i;

	m_pDS = NULL;
	m_pPriBuf = NULL;
	m_pSecBuf = NULL;

	if (FAILED(DirectSoundCreate(NULL, &m_pDS, NULL)))
		return;
	hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		hwnd = GetDesktopWindow();
	if (FAILED(m_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
	{
		m_pDS->Release();
		m_pDS = NULL;
		return;
	}
	if (FAILED(m_pDS->SetCooperativeLevel(hwnd, DSSCL_NORMAL)))
	{
		m_pDS->Release();
		m_pDS = NULL;
		return;
	}
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	desc.dwBufferBytes = 0;
	desc.dwReserved = 0;
	desc.lpwfxFormat = NULL;
	if (FAILED(m_pDS->CreateSoundBuffer(&desc, &m_pPriBuf, NULL)))
	{
		m_pDS->Release();
		m_pDS = NULL;
		return;
	}

	for (i = 0; i < 3; i++)
		m_phNotifyEvents[i] = CreateEvent(NULL, false, false, NULL);
	m_hEvent = CreateEvent(NULL, false, false, NULL);
	m_pBuffer = NULL;
	m_bPlaying = false;
	m_nTimer = 0;
}

CDirectSoundRenderer::~CDirectSoundRenderer()
{
	int i;

	if (m_pDS == NULL)
		return;

	Close();
	if (m_pPriBuf != NULL)
		m_pPriBuf->Release();
	if (m_pDS != NULL)
		m_pDS->Release();
	for (i = 0; i < 3; i++)
		CloseHandle(m_phNotifyEvents[i]);
	CloseHandle(m_hEvent);
}

bool CDirectSoundRenderer::Open(MediaInfo *pInfo)
{
	AudioRenderInfo *pAudioInfo;
	DSBUFFERDESC desc;
	LPDIRECTSOUNDNOTIFY pNotify; 
	DSBPOSITIONNOTIFY pPosNotify[3];
	int i;

	if (m_pDS == NULL)
		return false;
	Close();

	if (pInfo->nSize < sizeof(AudioRenderInfo))
		return false;

	pAudioRenderInfo = (AudioRenderInfo *)pInfo;
	InitWaveFormatExt(&m_WaveFormat, pAudioRenderInfo->nSampleRate, pAudioRenderInfo->nBitsPerSample, pAudioRenderInfo->nChannel);
	m_nBufferSize = pAudioRenderInfo->nSampleRate / 20 * ((pAudioRenderInfo->nBitsPerSample + 7) >> 3) * pAudioRenderInfo->nChannel;  //1/20 sec per block

	m_pPriBuf->SetFormat((WAVEFORMATEX *)&m_WaveFormat);
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;
	desc.dwBufferBytes = m_nBufferSize * 3;  //3 blocks
	desc.dwReserved = 0;
	desc.lpwfxFormat = (WAVEFORMATEX *)&m_WaveFormat;
	if (FAILED(m_pDS->CreateSoundBuffer(&desc, &m_pSecBuf, NULL)))
		return false;

	if (FAILED(m_pSecBuf->QueryInterface(IID_IDirectSoundNotify, (void **)&pNotify)))
		return false;
	for (i = 0; i < 3; i++)
	{
		pPosNotify[i].dwOffset = m_nBufferSize * (i + 1) - 1;  //end of block
		pPosNotify[i].hEventNotify = m_phNotifyEvents[i];
	}
	pNotify->SetNotificationPositions(3, pPosNotify);
	pNotify->Release();

	m_pBuffer = new char[m_nBufferSize];
	m_nBytesInBuffer = 0;
	m_nInputBufferNum = 0;
	m_nOutputBufferNum = 0;
	return true;
}

void CDirectSoundRenderer::Write(void *pBuffer, int nSize)
{
	int nCopy;
	void *pBuf;
	DWORD dwBufSize;

	if (m_pSecBuf == NULL || nSize == 0)
		return;

	if (!m_bPlaying)
	{
		m_bPlaying = true;
		m_pSecBuf->Lock(0, m_nBufferSize * 2, &pBuf, &dwBufSize, NULL, NULL, 0);  //two blocks 
		nCopy = nSize < m_nBufferSize * 2? nSize : m_nBufferSize * 2;
		memset(pBuf, 0, m_nBufferSize * 2 - nCopy);  //the head is silence
		memcpy((char *)pBuf + m_nBufferSize * 2 - nCopy, pBuffer, nCopy);  //the rest are current samples
		m_pSecBuf->Unlock(pBuf, dwBufSize, NULL, 0);
		m_pSecBuf->Play(0, 0, DSBPLAY_LOOPING);

		m_nTimer = timeSetEvent(50, 10, TimerProc, (DWORD)this, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
		pBuffer = (char *)pBuffer + m_nBufferSize * 2 - nCopy;
		nSize -= m_nBufferSize * 2 - nCopy;
		++m_nInputBufferNum;
	}

	while (nSize > 0)
	{
		if (m_nBytesInBuffer + nSize >= m_nBufferSize)  //there are enough samples to output
		{
			WaitForSingleObject(m_hEvent, INFINITE);  //wait for one block is completed playing
			m_pSecBuf->Lock(m_nBufferIndex * m_nBufferSize, m_nBufferSize, &pBuf, &dwBufSize, NULL, NULL, 0);
			memcpy(pBuf, m_pBuffer, m_nBytesInBuffer);  //copy buffered samples...
			memcpy((char *)pBuf + m_nBytesInBuffer, pBuffer, m_nBufferSize - m_nBytesInBuffer);  //...and current samples
			m_pSecBuf->Unlock(pBuf, dwBufSize, NULL, 0);

			pBuffer = (char *)pBuffer + m_nBufferSize - m_nBytesInBuffer;
			nSize -= m_nBufferSize - m_nBytesInBuffer;
			m_nBytesInBuffer = 0;
			++m_nInputBufferNum;
		}
		else
		{
			memcpy(m_pBuffer + m_nBytesInBuffer, pBuffer, nSize);  //save the rest samples in the queue
			m_nBytesInBuffer += nSize;
			break;
		}
	}
}

void CDirectSoundRenderer::Flush(bool bWait)
{
	char *pBuffer;

	if (m_pSecBuf == NULL)
		return;

	if (bWait)
	{
		if (m_nBytesInBuffer > 0)
		{
			pBuffer = new char[m_nBufferSize - m_nBytesInBuffer];
			memset(pBuffer, m_WaveFormat.Format.wBitsPerSample == 8? 0x80 : 0, m_nBufferSize - m_nBytesInBuffer);
			Write(pBuffer, m_nBufferSize - m_nBytesInBuffer);  //write some silence so that the samples are enough to output
			delete[] pBuffer;
		}
		if (m_nInputBufferNum > 0)
		{
			WaitForSingleObject(m_hEvent, INFINITE);
			WaitForSingleObject(m_hEvent, INFINITE);
		}
	}
	else
		m_nBytesInBuffer = 0;  //clear the buffer
}

void CDirectSoundRenderer::Close(void)
{
	if (m_nTimer != 0)
	{
		timeKillEvent(m_nTimer);
		m_nTimer = 0;
	}
	if (m_pSecBuf != NULL)
	{
		m_pSecBuf->Stop();
		m_pSecBuf->Release();
		m_pSecBuf = NULL;
	}
	delete[] m_pBuffer;
	m_pBuffer = NULL;
	m_bPlaying = false;
}

void CALLBACK CDirectSoundRenderer::TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CDirectSoundRenderer *pThis;

	pThis = (CDirectSoundRenderer *)dwUser;
	pThis->TimerCallback();
}

void CDirectSoundRenderer::TimerCallback(void)
{
	HRESULT hr;
	void *pBuf;
	DWORD dwBufSize;

	hr = WaitForMultipleObjects(3, m_phNotifyEvents, false, 0);
	if (hr >= WAIT_OBJECT_0 && hr < WAIT_OBJECT_0 + 3)  //complete playing one block
	{
		m_nBufferIndex = (hr - WAIT_OBJECT_0 + 2) % 3;  //the next of next block
		if (m_nOutputBufferNum < m_nInputBufferNum)
			++m_nOutputBufferNum;
		if (m_nOutputBufferNum >= m_nInputBufferNum)  //there is no input samples
		{
			m_pSecBuf->Lock(m_nBufferIndex * m_nBufferSize, m_nBufferSize, &pBuf, &dwBufSize, NULL, NULL, 0);
			memset(pBuf, m_WaveFormat.Format.wBitsPerSample == 8? 0x80 : 0, dwBufSize);  //silence
			m_pSecBuf->Unlock(pBuf, dwBufSize, NULL, 0);
		}
		SetEvent(m_hEvent);
	}
}
