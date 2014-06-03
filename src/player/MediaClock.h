#ifndef _MEDIACLOCK_H_
#define _MEDIACLOCK_H_

#include "Thread.h"

class CMediaClock
{
public:
	enum
	{
		EVENT_NUM = 10
	};
public:
	CMediaClock();
	~CMediaClock();
	void Pause(void);
	void Continue(void);
	void Reset(void);
	int GetTime(void);
	void SetTime(int nTime);
	void WaitFor(int nTime);
protected:
	bool m_bPaused;
	int m_nStart, m_nTick;
private:
	CWaitCondition *m_pWait[EVENT_NUM];
	int m_nIndex;
	CMutex *m_pMutex;
};

#endif
