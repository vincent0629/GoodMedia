#include "MediaPlayer.h"
#include "DataSourceFactory.h"
#include "DecoderFactory.h"
#include "RendererFactory.h"
#include "DIBRenderer.h"
#include "TextRenderer.h"
#include "FileRenderer.h"
#include "platform.h"

#define WM_PLAY_COMPLETED (WM_APP + 1)

CMediaPlayer::CMediaPlayer(HWND hWndVideo)
{
	m_hWndVideo = hWndVideo;
	m_pListener = NULL;
	m_pConfig = new CMediaConfig();
	m_pDataSource = NULL;
	m_pGraph = new CSourceGraph();
	m_pGraph->AddFormatChangedListener(this);
	m_nThreadNum = 0;
	m_pClock = new CMediaClock();
	m_nState = STATE_STOP;
	m_pMutex = new CMutex();
	m_pSemaphore = NULL;
	m_pOSDRenderer = NULL;
	memset(m_pThreadParams, 0, sizeof(m_pThreadParams));

	if (m_hWndVideo != NULL)
	{
		m_pWndVideo = new CMessageWindow(m_hWndVideo);
		m_pWndVideo->SetWindowListener(this);
	}
	else
		m_pWndVideo = NULL;
	m_pWndHidden = new CMessageWindow();  //create a hidden window to receive message
	m_pWndHidden->SetWindowListener(this);
}

CMediaPlayer::~CMediaPlayer()
{
	Close();
	delete m_pGraph;
	delete m_pClock;
	delete m_pMutex;
	delete m_pConfig;
	delete m_pWndVideo;
	delete m_pWndHidden;
}

void CMediaPlayer::SetPlayerListener(IPlayerListener *pListener)
{
	m_pListener = pListener;
}

bool CMediaPlayer::LoadConfig(const TCHAR *path)
{
	return m_pConfig->Load(path);
}

bool CMediaPlayer::Open(const TCHAR *path)
{
	int i;
	bool bClock;
	ThreadParam *pParam;
	VideoRenderInfo *pVideoInfo;
	int nWidth, nHeight;

	Close();

	m_pDataSource = CDataSourceFactory::Create(path);
	m_pDataSource->Open(path);
	if (_tcsncmp(path, _T("acap://"), 7) == 0)
		m_pGraph->Open(m_pDataSource, MEDIA_TYPE_PCM);  //open file, may callback FormatChanged
	else
		m_pGraph->Open(m_pDataSource);  //open file, may callback FormatChanged
	if (m_nThreadNum == 0)  //there is no media stream supported
	{
		delete m_pDataSource;
		m_pDataSource = NULL;
		return false;
	}

	bClock = false;
	pVideoInfo = NULL;
	for (i = 0; i < m_nThreadNum; i++)
	{
		pParam = m_pThreadParams + i;
		if ((pParam->type == RENDER_TYPE_AUDIO || pParam->type == RENDER_TYPE_SPDIF) && !bClock)
		{
			pParam->bReferenceClock = true;
			bClock = true;
		}
		else
			pParam->bReferenceClock = false;
		if (pParam->type == RENDER_TYPE_VIDEO && pVideoInfo == NULL)
			pVideoInfo = (VideoRenderInfo *)&pParam->info;
	}
	if (m_pListener != NULL)
		if (pParam->type == RENDER_TYPE_MIDI)
			m_pListener->VideoSizeChanged(this, 400, 30);  //for lyrics
		else if (pVideoInfo == NULL)
			m_pListener->VideoSizeChanged(this, 0, 0);  //notify there is no video
		else
		{
			nWidth = pVideoInfo->nWidth;
			nHeight = pVideoInfo->nHeight;
			if (pVideoInfo->fPixelAspectRatio > 1.0f)
				nHeight = (int)(nHeight * pVideoInfo->fPixelAspectRatio + 0.5f);
			else if (pVideoInfo->fPixelAspectRatio < 1.0f)
				nWidth = (int)(nWidth / pVideoInfo->fPixelAspectRatio + 0.5f);
			m_pListener->VideoSizeChanged(this, nWidth, nHeight);  //notify video size
		}
	m_pSemaphore = new CSemaphore(m_nThreadNum);

	return true;
}

void CMediaPlayer::Close(void)
{
	int i;
	ThreadParam *pParam;

	Stop();

	m_pGraph->Close();
	delete m_pDataSource;
	m_pDataSource = NULL;
	for (i = 0; i < m_nThreadNum; i++)
	{
		pParam = m_pThreadParams + i;
		delete pParam->pDecoder;
		pParam->pDecoder = NULL;
		delete pParam->pRenderer;
		pParam->pRenderer = NULL;
	}
	delete m_pOSDRenderer;
	m_pOSDRenderer = NULL;
	m_nThreadNum = 0;
	delete m_pSemaphore;
	m_pSemaphore = NULL;
}

void CMediaPlayer::Play(void)
{
	int i;

	if (m_nState == STATE_PLAY)
		return;

	if (m_nState == STATE_STOP)
	{
		m_nState = STATE_PLAY;
		for (i = 0; i < m_nThreadNum; i++)
			OpenRenderer(m_pThreadParams + i);
		for (i = 0; i < m_nThreadNum; i++)
		{
			m_pPlayThreads[i] = new CThread(this);
			m_pPlayThreads[i]->Start(m_pThreadParams + i);  //start play thread
		}
		m_pWaitThread = new CThread(this);
		m_pWaitThread->Start(NULL);  //start wait thread
	}
	else if (m_nState == STATE_PAUSE)
	{
		m_nState = STATE_PLAY;
		for (i = 0; i < m_nThreadNum; i++)  //continue all play threads
			m_pSemaphore->Release();
		if (m_pOSDRenderer != NULL)
			m_pOSDRenderer->Flush(false);  //hide OSD
	}
	m_pClock->Continue();  //start clock
}

void CMediaPlayer::Pause(void)
{
	int i;

	if (m_nState != STATE_PLAY)
		return;

	m_nState = STATE_PAUSE;
	m_pClock->Pause();  //pause clock
	for (i = 0; i < m_nThreadNum; i++)  //pause all play threads
		m_pSemaphore->Wait();
	for (i = 0; i < m_nThreadNum; i++)
		if (m_pThreadParams[i].type == RENDER_TYPE_MIDI)
			m_pThreadParams[i].pRenderer->Flush(false);  //MIDI renderer must be flushed to turn off all notes

	ShowOSD();
}

void CMediaPlayer::Stop(void)
{
	int nState, i;

	if (m_nState == STATE_STOP)
		return;

	nState = m_nState;
	m_nState = STATE_STOP;
	if (nState == STATE_PAUSE)
		for (i = 0; i < m_nThreadNum; i++)  //continue all play threads if they are paused
			m_pSemaphore->Release();
	m_pClock->Reset();  //cause the WaitFor function to return immediately
	m_pWaitThread->Join();  //wait for all play threads to complete
	delete m_pWaitThread;
	m_pClock->SetTime(m_pGraph->SeekTime(0));

	if (m_hWndVideo != NULL)
	{
		if (m_pOSDRenderer != NULL)
			m_pOSDRenderer->Flush(false);  //hide OSD
		InvalidateRect(m_hWndVideo, NULL, FALSE);  //repaint last frame
	}
}

int CMediaPlayer::GetDuration(void)
{
	return m_pGraph->GetDuration();
}

int CMediaPlayer::GetTime(void)
{
	int nTime, nDuration;

	if (m_nState == STATE_STOP)
		return 0;
	nTime = m_pClock->GetTime();
	nDuration = m_pGraph->GetDuration();
	if (nDuration > 0 && nTime > nDuration)
		nTime = nDuration;
	return nTime;
}

void CMediaPlayer::SeekTime(int nTime)
{
	int i;
	ThreadParam *pParam;

	m_pClock->Reset();  //cause the WaitFor function to return immediately
	m_pMutex->Lock();
	m_pClock->SetTime(m_pGraph->SeekTime(nTime));

	if (m_nState == STATE_PLAY)
	{
		for (i = 0; i < m_nThreadNum; i++)
		{
			pParam = m_pThreadParams + i;
			if (pParam->pDecoder != NULL)
				pParam->pDecoder->Flush();  //bug? it is possible that the decoder is decoding in PlayFunc
			if (pParam->pRenderer != NULL)
				pParam->pRenderer->Flush(false);  //bug? it is possible that the renderer is rendering in PlayFunc
		}
		m_pClock->Continue();  //start clock
	}
	m_pMutex->Unlock();
}

bool CMediaPlayer::OpenRenderer(ThreadParam *pParam)
{
	FileRenderInfo fileInfo;

	switch (pParam->type)
	{
		case RENDER_TYPE_VIDEO:
			if (m_pOSDRenderer != NULL)
			{
				pParam->info.nSize = sizeof(TextRenderInfo);
				((TextRenderInfo *)&pParam->info)->hwnd = m_hWndVideo;
				m_pOSDRenderer->Open(&pParam->info);
			}
			pParam->info.nSize = sizeof(DIBRenderInfo);
			((DIBRenderInfo *)&pParam->info)->hwnd = m_hWndVideo;
		case RENDER_TYPE_AUDIO:
		case RENDER_TYPE_SPDIF:
		case RENDER_TYPE_MIDI:
			return pParam->pRenderer->Open(&pParam->info);
		case RENDER_TYPE_FILE:
			fileInfo.nSize = sizeof(FileRenderInfo);
			fileInfo.path = m_pConfig->GetString(_T("player.filerenderer.path"), _T("C:\\dump.pcm"));
			return pParam->pRenderer->Open(&fileInfo);
		default:
			return false;
	}  //switch
}

void CMediaPlayer::PlayFunc(ThreadParam *pParam)
{
	CMediaSource *pSource;
	CMediaDecoder *pDecoder;
	CMediaRenderer *pRenderer;
	int nTime;
	bool bDrop;
	int nReadSize, nSrcSize, nDecSize, nRndSize;
	unsigned char *pSrcFrame, *pDecFrame, *pRndFrame;

	pSource = pParam->pSource;
	pDecoder = pParam->pDecoder;
	pRenderer = pParam->pRenderer;
	nTime = 0;
	bDrop = false;
	if (pDecoder != NULL)
	{
		nReadSize = pDecoder->GetInputSize();
		pSrcFrame = new unsigned char[nReadSize];
		nDecSize = pDecoder->GetOutputSize();
		pDecFrame = new unsigned char[nDecSize];
		pRndFrame = pDecFrame;
	}
	else
	{
		nReadSize = 0x1000;
		pSrcFrame = new unsigned char[nReadSize];
		pDecFrame = NULL;
		pRndFrame = pSrcFrame;
	}

	for (;;)
	{
		m_pSemaphore->Wait();  //when paused, this thread will be blocked here
		if (m_nState == STATE_STOP)
		{
			m_pSemaphore->Release();
			break;
		}

		if (!pParam->bReferenceClock)
			if (nTime + 100 < m_pClock->GetTime())  //time stamp behind the clock too much
				bDrop = true;
		m_pMutex->Lock();  //only one thread can read data from media source
		nSrcSize = pSource->ReadData(pParam->nIndex, bDrop? NULL : pSrcFrame, nReadSize);  //read a frame
		m_pMutex->Unlock();
		if (nSrcSize == 0)  //no more frame to render
		{
			pRenderer->Flush(true);
			m_pSemaphore->Release();
			break;
		}

		nTime = pSource->GetOutputTime(pParam->nIndex);
		if (bDrop)  //drop this frame
			bDrop = false;
		else
		{
			if (pDecoder != NULL)
			{
				if (pParam->type != RENDER_TYPE_SPDIF)
					nDecSize = pDecoder->Decode(pSrcFrame, nSrcSize, pDecFrame);
				else
					nDecSize = pDecoder->SPDIF(pSrcFrame, nSrcSize, pDecFrame);
			}
			else
				nDecSize = nSrcSize;
			nRndSize = nDecSize;

			if (!pParam->bReferenceClock)
				m_pClock->WaitFor(nTime);  //wait until this frame can be rendered
			pRenderer->Write(pRndFrame, nRndSize);  //render a frame
			if (pParam->bReferenceClock && nTime > 0)
				m_pClock->SetTime(nTime);  //update clock
		}

		m_pSemaphore->Release();
	}  //while

	if (pParam->pDecoder)
		pParam->pDecoder->Flush();
	delete[] pSrcFrame;
	delete[] pDecFrame;
}

void CMediaPlayer::WaitFunc(void)
{
	int i;

	for (i = 0; i < m_nThreadNum; i++)
	{
		m_pPlayThreads[i]->Join();  //wait for play thread to complete
		delete m_pPlayThreads[i];  //close play thread
		if (m_pThreadParams[i].type != RENDER_TYPE_VIDEO)  //don't close video renderer, so that the last frame can be painted
			m_pThreadParams[i].pRenderer->Close();  //close renderer
	}
	if (m_nState == STATE_PLAY)  //play completed
		m_pWndHidden->PostMessage(WM_PLAY_COMPLETED, 0, 0);  //close this thread
}

void CMediaPlayer::ShowOSD(void)
{
	TextRenderData textData;

	if (m_pOSDRenderer != NULL)
	{
		textData.x = 10;
		textData.y = 10;
		textData.nColor = 0x00FF00;
		textData.pText = _T("Pause");
		m_pOSDRenderer->Write(&textData, sizeof(TextRenderData));  //show OSD
	}
}

//implement IFormatChangedListener
void CMediaPlayer::FormatChanged(void *pObj, CMediaSource *pSource, int nIndex)
{
	ThreadParam *pParam;
	MediaType type;
	char pBuffer[256];
	MediaInfo *pInfo;
	RenderType rtype;
	int nAudioRenderer;
	int i;

	if (m_nThreadNum == PLAY_THREAD_NUM)  //too many play threads
		return;

	pParam = m_pThreadParams + m_nThreadNum;
	type = pSource->GetOutputType(nIndex);
	switch (type)
	{
		case MEDIA_TYPE_BITMAP:
		case MEDIA_TYPE_MPEGVIDEO:
			pParam->type = RENDER_TYPE_VIDEO;
			break;
		case MEDIA_TYPE_MPEGAUDIO:
		case MEDIA_TYPE_PCM:
			pParam->type = RENDER_TYPE_AUDIO;
			break;
		case MEDIA_TYPE_AC3:
			pParam->type = RENDER_TYPE_SPDIF;
			break;
		case MEDIA_TYPE_MIDIEVENT:
			pParam->type = RENDER_TYPE_MIDI;
			break;
		default:  //stream type is not supported
			return;
	}

	for (i = 0; i < m_nThreadNum; i++)
		if (m_pThreadParams[i].type == pParam->type)  //there is already one stream with the same render type
			return;

	pInfo = (MediaInfo *)pBuffer;
	pInfo->nSize = sizeof(pBuffer);
	if (!pSource->GetOutputInfo(nIndex, pInfo))
		return;

	pParam->pDecoder = CDecoderFactory::Create(type);
	if (pParam->pDecoder != NULL)
	{
		if (!pParam->pDecoder->SetInputInfo(type, pInfo))  //decoder not supports
		{
			delete pParam->pDecoder;
			pParam->pDecoder = NULL;
			return;
		}

		pInfo->nSize = sizeof(pBuffer);
		pParam->pDecoder->GetOutputInfo(pInfo);
		memcpy(&pParam->info, pInfo, pInfo->nSize);
	}

	rtype = pParam->type;
	if (rtype == RENDER_TYPE_AUDIO)
	{
		nAudioRenderer = m_pConfig->GetInt(_T("player.audio.renderer"), 0);
		if (nAudioRenderer != 0)
			rtype = RENDER_TYPE_FILE;
	}
	pParam->pRenderer = CRendererFactory::Create(rtype);
	if (pParam->pRenderer == NULL)
		return;
	if (rtype == RENDER_TYPE_VIDEO)
		m_pOSDRenderer = CRendererFactory::Create(RENDER_TYPE_TEXT);
	else if (rtype == RENDER_TYPE_MIDI)
		pParam->pRenderer->SetListener(this);

	pParam->pSource = pSource;
	pParam->nIndex = nIndex;
	pParam->pPlayer = this;
	++m_nThreadNum;
}

//implement IRunnable
void CMediaPlayer::Run(void *pData)
{
	if (pData == NULL)
		WaitFunc();
	else
		PlayFunc((ThreadParam *)pData);
}

//implement IWindowListener
LRESULT CMediaPlayer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (uMsg)
	{
		case WM_PAINT:
			if (m_nState != STATE_PLAY)
			{
				for (i = 0; i < m_nThreadNum; i++)
					if (m_pThreadParams[i].type == RENDER_TYPE_VIDEO)
					{
						m_pThreadParams[i].pRenderer->Write(NULL, 0);  //repaint the last frame
						break;
					}
				if (m_nState == STATE_PAUSE)
					ShowOSD();
				return 0;
			}
			break;
		//custom message
		case WM_PLAY_COMPLETED:
			Stop();
			if (m_pListener != NULL)
				m_pListener->PlayCompleted(this);
			return 0;
	}  //switch
	return 1;
}

//implement IMediaRendererListener
void CMediaPlayer::MediaRendered(CMediaRenderer *pRenderer, void *pData, int nSize)
{
	TCHAR pLyrics[0x100];

#ifdef UNICODE
	pLyrics[0] = _T('\0');
#else
	memcpy(pLyrics, pData, nSize);
	pLyrics[nSize] = '\0';
#endif
	SetWindowText(m_hWndVideo, pLyrics);
}
