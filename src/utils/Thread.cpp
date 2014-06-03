#include "Thread.h"

CThread::CThread(IRunnable *pRunnable)
{
	m_pRunnable = pRunnable;
	m_hThread = 0;
	m_pData = NULL;
}

CThread::~CThread()
{
	Join();
}

void CThread::Start(void *pData)
{
	if (m_hThread)
		return;

	m_pData = pData;
#ifdef _WIN32
	m_hThread = (HANDLE)CreateThread(NULL, 0, ThreadProc, this, 0, NULL);  //start thread
#else
	pthread_create(&m_hThread, NULL, ThreadProc, this);  //start thread
#endif
}

void CThread::Join(void)
{
	if (m_hThread)
	{
#ifdef _WIN32
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
#else
		pthread_join(m_hThread, NULL);
#endif
		m_hThread = 0;
	}
}

#ifdef _WIN32
DWORD WINAPI CThread::ThreadProc(void *pParam)
#else
void *CThread::ThreadProc(void *pParam)
#endif
{
	CThread *pThis;

	pThis = (CThread *)pParam;
	pThis->m_pRunnable->Run(pThis->m_pData);
	return 0;
}

CMutex::CMutex()
{
#ifdef _WIN32
	m_handle = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutex_init(&m_handle, NULL);
#endif
}

CMutex::~CMutex()
{
#ifdef _WIN32
	CloseHandle(m_handle);
#else
	pthread_mutex_destroy(&m_handle);
#endif
}

void CMutex::Lock()
{
#ifdef _WIN32
	WaitForSingleObject(m_handle, INFINITE);
#else
	pthread_mutex_lock(&m_handle);
#endif
}

void CMutex::Unlock()
{
#ifdef _WIN32
	ReleaseMutex(m_handle);
#else
	pthread_mutex_unlock(&m_handle);
#endif
}

CWaitCondition::CWaitCondition()
{
#ifdef _WIN32
	m_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	pthread_cond_init(&m_handle, NULL);
	pthread_mutex_init(&m_mutex, NULL);
#endif
}

CWaitCondition::~CWaitCondition()
{
#ifdef _WIN32
	CloseHandle(m_handle);
#else
	pthread_cond_destroy(&m_handle);
#endif
}

void CWaitCondition::Wait(void)
{
#ifdef _WIN32
	WaitForSingleObject(m_handle, INFINITE);
#else
	pthread_cond_wait(&m_handle, &m_mutex);
#endif
}

void CWaitCondition::Wake(void)
{
#ifdef _WIN32
	SetEvent(m_handle);
#else
	pthread_cond_broadcast(&m_handle);
#endif
}

CSemaphore::CSemaphore(int nCount)
{
#ifdef _WIN32
	m_handle = CreateSemaphore(NULL, nCount, nCount, NULL);
#else
#endif
}

CSemaphore::~CSemaphore()
{
#ifdef _WIN32
	CloseHandle(m_handle);
#else
#endif
}

void CSemaphore::Wait(void)
{
#ifdef _WIN32
	WaitForSingleObject(m_handle, INFINITE);
#else
#endif
}

void CSemaphore::Release(void)
{
#ifdef _WIN32
	ReleaseSemaphore(m_handle, 1, NULL);
#else
#endif
}
