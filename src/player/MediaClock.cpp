#include "MediaClock.h"

static void CALLBACK TimeCallback(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	((CWaitCondition *)dwUser)->Wake();
}

CMediaClock::CMediaClock()
{
	int i;

	for (i = 0; i < EVENT_NUM; i++)
		m_pWait[i] = new CWaitCondition();
	m_nIndex = 0;
	m_pMutex = new CMutex();
	m_bPaused = true;
	SetTime(0);
}

CMediaClock::~CMediaClock()
{
	int i;

	for (i = 0; i < EVENT_NUM; i++)
	{
		m_pWait[i]->Wake();
		delete m_pWait[i];
	}
	delete m_pMutex;
}

void CMediaClock::Pause(void)
{
	if (!m_bPaused)
	{
		m_nTick += timeGetTime() - m_nStart;
		m_bPaused = true;
	}
}

void CMediaClock::Continue(void)
{
	if (m_bPaused)
	{
		m_nStart = timeGetTime();
		m_bPaused = false;
	}
}

void CMediaClock::Reset(void)
{
	int i;

	m_pMutex->Lock();
	for (i = 0; i < EVENT_NUM; i++)
		m_pWait[i]->Wake();
	m_pMutex->Unlock();

	m_bPaused = true;
	SetTime(0);
}

void CMediaClock::SetTime(int nTime)
{
	m_nStart = timeGetTime();
	m_nTick = nTime;
}

int CMediaClock::GetTime(void)
{
	return m_bPaused? m_nTick : m_nTick + timeGetTime() - m_nStart;
}

void CMediaClock::WaitFor(int nTime)
{
	unsigned int nTimer;

	nTime -= GetTime();
	if (nTime > 0)
	{
		m_pMutex->Lock();
		if (++m_nIndex >= EVENT_NUM)
			m_nIndex = 0;
		m_pMutex->Unlock();
		nTimer = timeSetEvent(nTime, 1, TimeCallback, (DWORD)m_pWait[m_nIndex], TIME_ONESHOT | TIME_CALLBACK_FUNCTION);
		m_pWait[m_nIndex]->Wait();
		timeKillEvent(nTimer);
	}
}
