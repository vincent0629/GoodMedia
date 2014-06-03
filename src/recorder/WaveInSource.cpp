#include "WaveInSource.h"
#include "platform.h"
#include <string.h>

CWaveInSource::CWaveInSource()
{
	m_hWaveIn = NULL;
	m_pBuffer = NULL;
	m_nBufferSize = 0;
}

CWaveInSource::~CWaveInSource()
{
	Close();
}

bool CWaveInSource::Open(const TCHAR *path)
{
	TCHAR ppath[20], *str;
	int nSampleRate, nBitsPerSample, nChannel;

	if (_tcsncmp(path, _T("acap://"), 7))
		return false;

	_tcscpy(ppath, path + 7);
	str = _tcstok(ppath, _T(","));
	if (str == NULL)
		return false;
	nSampleRate = _ttoi(str);
	str = _tcstok(NULL, _T(","));
	if (str == NULL)
		return false;
	nBitsPerSample = _ttoi(str);
	str = _tcstok(NULL, _T(","));
	if (str == NULL)
		return false;
	nChannel = _ttoi(str);
	return Open(nSampleRate, nBitsPerSample, nChannel);
}

bool CWaveInSource::Open(int nSampleRate, int nBitsPerSample, int nChannel)
{
	int i;

	Close();

	m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat.nSamplesPerSec = nSampleRate;
	m_WaveFormat.nChannels = nChannel;
	m_WaveFormat.wBitsPerSample = nBitsPerSample;
	m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels * ((m_WaveFormat.wBitsPerSample + 7) >> 3);
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;
	m_WaveFormat.cbSize = 0;

	if (waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_WaveFormat, (DWORD)waveInProc, (DWORD)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)  //open wave in
		return false;

	m_nRecordIndex = 0;
	m_nDataNum = 0;
	m_bStarted = false;
	m_nBufferSize = nSampleRate / 10 * ((nBitsPerSample + 7) >> 3) * nChannel;  //buffer size is 1/10 second
	m_pBuffer = new char[m_nBufferSize * BUFFER_NUM];
	for (i = 0; i < BUFFER_NUM; i++)  //initialize wave header
	{
		memset(m_WaveHdr + i, 0, sizeof(WAVEHDR));
		m_WaveHdr[i].dwBufferLength = m_nBufferSize;
		m_WaveHdr[i].lpData = m_pBuffer + i * m_nBufferSize;
		waveInPrepareHeader(m_hWaveIn, m_WaveHdr + i, sizeof(WAVEHDR));
	}
	m_hEvent = CreateEvent(NULL, false, false, NULL);
	return true;
}

int CWaveInSource::ReadData(void *pBuffer, int nSize)
{
	char *pcBuffer;
	int i, n, nRest;

	if (m_hWaveIn == 0)
		return 0;

	if (m_nDataNum == 0 && !m_bStarted)
	{
		m_bStarted = true;
		m_nRecordIndex = 0;
		m_nReadIndex = 0;

		for (i = 0; i < 2; i++)
		{
			waveInAddBuffer(m_hWaveIn, m_WaveHdr + m_nRecordIndex, sizeof(WAVEHDR));
			++m_nRecordIndex;
		}
		waveInStart(m_hWaveIn);
	}

	pcBuffer = (char *)pBuffer;
	nRest = nSize;
	while (nRest > 0)
	{
		if (m_nDataNum == 0)
			WaitForSingleObject(m_hEvent, INFINITE);

		n = m_nBufferSize - m_nReadIndex % m_nBufferSize;
		if (n > nRest)
			n = nRest;
		memcpy(pcBuffer, m_pBuffer + m_nReadIndex, n);
		m_nReadIndex += n;
		if (m_nReadIndex >= BUFFER_NUM * m_nBufferSize)
			m_nReadIndex -= BUFFER_NUM * m_nBufferSize;
		pcBuffer += n;
		nRest -= n;

		if (nRest > 0 || m_nReadIndex % m_nBufferSize == 0)
		{
			ResetEvent(m_hEvent);
			--m_nDataNum;
		}
	}

	return nSize;
}

void CWaveInSource::Close(void)
{
	int i;

	if (m_hWaveIn == NULL)
		return;

	waveInStop(m_hWaveIn);
	for (i = 0; i < BUFFER_NUM; i++)
		waveInUnprepareHeader(m_hWaveIn, m_WaveHdr + i, sizeof(WAVEHDR));
	waveInClose(m_hWaveIn);
	m_hWaveIn = NULL;

	CloseHandle(m_hEvent);
	m_hEvent = NULL;
	delete[] m_pBuffer;
	m_pBuffer = NULL;
	m_nBufferSize = 0;
}

void CALLBACK CWaveInSource::waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == WIM_DATA)
		((CWaveInSource *)dwInstance)->WaveInProc();
}

void CWaveInSource::WaveInProc(void)
{
	if (m_nDataNum < BUFFER_NUM)
		++m_nDataNum;
	else
	{
		m_nReadIndex /= m_nBufferSize;
		if (++m_nReadIndex == BUFFER_NUM)
			m_nReadIndex = 0;
		m_nReadIndex *= m_nBufferSize;
	}
	SetEvent(m_hEvent);

	waveInAddBuffer(m_hWaveIn, m_WaveHdr + m_nRecordIndex, sizeof(WAVEHDR));
	if (++m_nRecordIndex == BUFFER_NUM)
		m_nRecordIndex = 0;
}
