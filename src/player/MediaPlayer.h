#ifndef _MEDIAPLAYER_H_
#define _MEDIAPLAYER_H_

#include "Thread.h"
#include "MessageWindow.h"
#include "PlayerListener.h"
#include "MediaConfig.h"
#include "SourceGraph.h"
#include "MediaDecoder.h"
#include "MediaClock.h"
#include "MediaRendererListener.h"

class CMediaPlayer : public IFormatChangedListener, IRunnable, IWindowListener, IMediaRendererListener
{
public:
	enum
	{
		PLAY_THREAD_NUM = 3
	};
	typedef enum
	{
		STATE_STOP,
		STATE_PLAY,
		STATE_PAUSE
	} PlayerState;
	typedef struct
	{
		RenderType type;
		CMediaSource *pSource;
		int nIndex;
		CMediaDecoder *pDecoder;
		CMediaRenderer *pRenderer;
		CMediaPlayer *pPlayer;
		bool bReferenceClock;
		MediaInfo info;
		char reserved[64];
	} ThreadParam;
public:
	CMediaPlayer(HWND hWndVideo);
	virtual ~CMediaPlayer();
	void SetPlayerListener(IPlayerListener *pListener);
	virtual bool LoadConfig(const TCHAR *path);
	virtual bool Open(const TCHAR *path);
	virtual void Close(void);
	virtual void Play(void);
	virtual void Pause(void);
	virtual void Stop(void);
	virtual int GetDuration(void);
	virtual int GetTime(void);
	virtual void SeekTime(int nTime);
	void FormatChanged(void *pObj, CMediaSource *pSource, int nIndex);
	void Run(void *pData);
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void MediaRendered(CMediaRenderer *pRenderer, void *pData, int nSize);
private:
	HWND m_hWndVideo;
	CMessageWindow *m_pWndVideo, *m_pWndHidden;
	IPlayerListener *m_pListener;
	CMediaConfig *m_pConfig;
	CDataSource *m_pDataSource;
	CSourceGraph *m_pGraph;
	CMediaRenderer *m_pOSDRenderer;
	ThreadParam m_pThreadParams[PLAY_THREAD_NUM];
	CThread *m_pPlayThreads[PLAY_THREAD_NUM], *m_pWaitThread;
	int m_nThreadNum;
	CMediaClock *m_pClock;
	PlayerState m_nState;
	CSemaphore *m_pSemaphore;
	CMutex *m_pMutex;
	int m_nSeekTime;

	bool OpenRenderer(ThreadParam *pParam);
	void PlayFunc(ThreadParam *pParam);
	void WaitFunc(void);
	void ShowOSD(void);
};

#endif
