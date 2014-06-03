#ifndef _THREAD_H_
#define _THREAD_H_

#include "Runnable.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

class CThread
{
public:
	CThread(IRunnable *pRunnable);
	~CThread();
	void Start(void *pData);
	void Join(void);
private:
	IRunnable *m_pRunnable;
#ifdef _WIN32
	HANDLE m_hThread;
#else
	pthread_t m_hThread;
#endif
	void *m_pData;

#ifdef _WIN32
	static DWORD WINAPI ThreadProc(void *pParam);
#else
	static void *ThreadProc(void *pParam);
#endif
};

class CMutex
{
public:
	CMutex();
	~CMutex();
	void Lock();
	void Unlock();
private:
#ifdef _WIN32
	HANDLE m_handle;
#else
	pthread_mutex_t m_handle;
#endif
};

class CWaitCondition
{
public:
	CWaitCondition();
	~CWaitCondition();
	void Wait(void);
	void Wake(void);
private:
#ifdef _WIN32
	HANDLE m_handle;
#else
	pthread_cond_t m_handle;
	pthread_mutex_t m_mutex;
#endif
};

class CSemaphore
{
public:
	CSemaphore(int nCount);
	~CSemaphore();
	void Wait(void);
	void Release(void);
private:
#ifdef _WIN32
	HANDLE m_handle;
#else
	sem_t *m_sem;
#endif
};

#endif
