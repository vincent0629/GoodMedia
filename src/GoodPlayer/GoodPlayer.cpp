#include "GoodPlayer.h"
#include "MediaPlayer.h"
#include "WindowsDataSourceFactory.h"
#include "DefaultMediaSourceFactory.h"
#include "DefaultDecoderFactory.h"
#include "WindowsRendererFactory.h"
#include <commdlg.h>
#include <tchar.h>

//#define STACKWALKER
#ifdef STACKWALKER
#include "D:\work\cpp\Stackwalker.h"
#endif

static HINSTANCE g_hInstance;
static HWND g_hMain;  //main window
static HWND g_btOpen, g_btPlay, g_btPause, g_btStop;  //button
static HWND g_stTime;  //text
static HWND g_sbTime;  //scrollbar
static HWND g_hPanel;  //panel
static CGoodPlayer *g_pGoodlayer;
static CMediaPlayer *g_pPlayer;
static int g_nDuration;
static bool g_bMoveTime;

#ifdef WINCE
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
int GetScrollPos(HWND hwnd, int nBar)
{
	SCROLLINFO info;

	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_POS;
	GetScrollInfo(g_sbTime, SB_CTL, &info);
	return info.nPos;
}
#endif

HWND CreateButton(HWND hParent, TCHAR *name, int x, int y, int width, int height)
{
	HWND hwnd;

	hwnd = CreateWindow(_T("BUTTON"), name, WS_CHILD | WS_VISIBLE | BS_CENTER | BS_TEXT, x, y, width, height, hParent, NULL, g_hInstance, NULL);
	return hwnd;
}

HWND CreateScrollBar(HWND hParent, int x, int y, int width, int height)
{
	HWND hwnd;

	hwnd = CreateWindow(_T("SCROLLBAR"), _T(""), WS_CHILD | WS_VISIBLE | SBS_HORZ, x, y, width, height, hParent, NULL, g_hInstance, NULL);
	return hwnd;
}

HWND CreateStatic(HWND hParent, TCHAR *name, int x, int y, int width, int height)
{
	HWND hwnd;

	hwnd = CreateWindow(_T("STATIC"), name, WS_CHILD | WS_VISIBLE | BS_CENTER, x, y, width, height, hParent, NULL, g_hInstance, NULL);
	return hwnd;
}

void UpdatePlayTime(int nTime)
{
	TCHAR str[10];

	_stprintf(str, _T("%02d:%02d"), nTime / 60000, nTime / 1000 % 60);
	SetWindowText(g_stTime, str);
	SetScrollPos(g_sbTime, SB_CTL, nTime / 100, true);  //set time bar position
}

void OnCreate(HWND hwnd)
{
	g_btOpen = CreateButton(hwnd, _T("V"), 0, 0, 20, 20);
	g_btPlay = CreateButton(hwnd, _T("|>"), 20, 0, 20, 20);
	g_btPause = CreateButton(hwnd, _T("||"), 40, 0, 20, 20);
	g_btStop = CreateButton(hwnd, _T("[]"), 60, 0, 20, 20);
	g_stTime = CreateStatic(hwnd, _T("00:00"), 0, 20, 60, 20);
	g_sbTime = CreateScrollBar(hwnd, 60, 20, 220, 20);
	g_hPanel = CreateStatic(hwnd, _T(""), 0, 40, 300, 200);
#ifndef WINCE
	DragAcceptFiles(hwnd, true);
#endif
	CWindowsDataSourceFactory::Register();
	CDefaultMediaSourceFactory::Register();
	CDefaultDecoderFactory::Register();
	CWindowsRendererFactory::Register();
	g_pGoodlayer = new CGoodPlayer();
	g_pPlayer = new CMediaPlayer(g_hPanel);
	g_pPlayer->SetPlayerListener(g_pGoodlayer);
	g_pPlayer->LoadConfig(_T("GoodPlayer.cfg"));
	g_nDuration = 0;

	EnableWindow(g_btPlay, false);
	EnableWindow(g_btPause, false);
	EnableWindow(g_btStop, false);
}

void OnPlay(void)
{
	g_pPlayer->Play();
	g_bMoveTime = true;  //start moving time bar position
	SetTimer(g_hMain, 1, 100, NULL);  //update play time periodically

	EnableWindow(g_btPlay, false);
	EnableWindow(g_btPause, true);
	EnableWindow(g_btStop, true);
}

void OpenMedia(TCHAR *path)
{
	SetWindowText(g_stTime, _T("00:00"));
	SetScrollPos(g_sbTime, SB_CTL, 0, false);  //move time bar position to 0
	if (!g_pPlayer->Open(path))
	{
		MessageBox(g_hMain, _T("File does not exist\nor is not supported"), _T("Error"), MB_OK);
		return;
	}
	g_nDuration = g_pPlayer->GetDuration() / 100;  //duration in 1/10 sec
	SetScrollRange(g_sbTime, SB_CTL, 0, g_nDuration, true);  //set time bar range

	OnPlay();  //start playing
}

#ifndef WINCE
void OnDropFiles(HDROP hDrop)
{
	char path[256];

	DragQueryFile(hDrop, 0, path, sizeof(path));
	DragFinish(hDrop);
	OpenMedia(path);
}
#endif

void OnOpen(void)
{
	OPENFILENAME file;
	TCHAR str[255] = {0};

	memset(&file, 0, sizeof(OPENFILENAME));
	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = g_hMain;
	file.hInstance = g_hInstance;
	file.lpstrFilter = _T("All Files\0*.*\0");
	file.lpstrFile = str;
	file.nMaxFile = 255;
	file.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	if (GetOpenFileName(&file))
		OpenMedia(file.lpstrFile);
}

void OnPause(void)
{
	KillTimer(g_hMain, 1);  //stop updating play time
	g_pPlayer->Pause();

	EnableWindow(g_btPlay, true);
	EnableWindow(g_btPause, false);
}

void OnStop(void)
{
	KillTimer(g_hMain, 1);  //stop updating play time
	g_pPlayer->Stop();

	EnableWindow(g_btPlay, true);
	EnableWindow(g_btPause, false);
	EnableWindow(g_btStop, false);
}

void OnCommand(HWND hwnd)
{
	if (hwnd == g_btOpen)
		OnOpen();
	else if (hwnd == g_btPlay)
		OnPlay();
	else if (hwnd == g_btPause)
		OnPause();
	else if (hwnd == g_btStop)
		OnStop();
}

void OnHScroll(HWND hwnd, WORD wRequest, DWORD wPos)
{
	int nPosition;
	TCHAR str[10];

	nPosition = GetScrollPos(g_sbTime, SB_CTL);
	switch (wRequest)
	{
		case SB_LEFT:
			nPosition = 0;
			break;
		case SB_RIGHT:
			nPosition = g_nDuration;
			break;
		case SB_LINELEFT:
			nPosition = nPosition > 1? nPosition - 1 : 0;  //1/10 sec
			break;
		case SB_LINERIGHT:
			++nPosition;  //1/10 sec
			break;
		case SB_PAGELEFT:
			nPosition = nPosition > 10? nPosition - 10 : 0;  //1 sec
			break;
		case SB_PAGERIGHT:
			nPosition += 10;  //1 sec
			break;
		case SB_THUMBTRACK:
			g_bMoveTime = false;
			return;
		case SB_THUMBPOSITION:
			nPosition = wPos;
			g_bMoveTime = true;
			break;
	}
	if (nPosition > g_nDuration)
		nPosition = g_nDuration;  //in 1/10 sec
	if (nPosition != GetScrollPos(g_sbTime, SB_CTL))  //scrollbar position is changed
	{
		g_pPlayer->SeekTime(nPosition * 100);  //seek to specified time
		_stprintf(str, _T("%02d:%02d"), nPosition / 600, nPosition / 10 % 60);
		SetWindowText(g_stTime, str);
		SetScrollPos(g_sbTime, SB_CTL, nPosition, true);
	}
}

void OnSize(HWND hwnd)
{
	POINT point = {0, 0};
	RECT rcClient, rcWindow;

	ClientToScreen(hwnd, &point);
	GetClientRect(hwnd, &rcClient);
	GetWindowRect(g_sbTime, &rcWindow);
	SetWindowPos(g_sbTime, 0, 0, 0, rcClient.right - rcWindow.left + point.x, rcWindow.bottom - rcWindow.top, SWP_NOMOVE | SWP_NOZORDER);  //resize scroll bar to fit duration
	GetWindowRect(g_hPanel, &rcWindow);
	SetWindowPos(g_hPanel, 0, 0, 0, rcClient.right, rcClient.bottom - rcWindow.top + point.y, SWP_NOMOVE | SWP_NOZORDER);  //resize panel to fit video
}

void OnTimer(HWND hwnd)
{
	if (g_bMoveTime)
		UpdatePlayTime(g_pPlayer->GetTime());
}

void OnClose(HWND hwnd)
{
	g_pPlayer->Close();
}

void CGoodPlayer::VideoSizeChanged(void *pObj, int nWidth, int nHeight)
{
	RECT rcWindow, rcClient;

	GetClientRect(g_hMain, &rcClient);
	if (nWidth == 0)  //there is no video
		nWidth = rcClient.right - rcClient.left;  //keep original width
	SetWindowPos(g_hPanel, 0, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOZORDER);  //resize panel to fit video
	GetWindowRect(g_hMain, &rcWindow);
	SetWindowPos(g_hMain, 0, 0, 0, nWidth + (rcWindow.right - rcWindow.left) - rcClient.right, nHeight + 40 + (rcWindow.bottom - rcWindow.top) - rcClient.bottom, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);  //resize window to fit panel
}

void CGoodPlayer::PlayCompleted(void *pObj)
{
	KillTimer(g_hMain, 1);  //stop updating play time
	UpdatePlayTime(g_pPlayer->GetDuration());
	EnableWindow(g_btPlay, true);
	EnableWindow(g_btPause, false);
	EnableWindow(g_btStop, false);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
			OnCreate(hwnd);
			break;
#ifndef WINCE
		case WM_DROPFILES:
			OnDropFiles((HDROP)wParam);
			break;
#endif
		case WM_COMMAND:
			OnCommand((HWND)lParam);
			break;
		case WM_HSCROLL:
			OnHScroll(hwnd, LOWORD(wParam), HIWORD(wParam));
			break;
		case WM_SIZE:
			OnSize(hwnd);
			break;
		case WM_TIMER:
			OnTimer(hwnd);
			break;
		case WM_CLOSE:
			OnClose(hwnd);
			delete g_pPlayer;
			delete g_pGoodlayer;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wndclass;
	RECT rect;
	int nWidth, nHeight;
	MSG msg;

#ifdef STACKWALKER
	InitAllocCheck();
#endif
	memset(&wndclass, 0, sizeof(WNDCLASS));
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WindowProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszClassName = _T("GoodPlayerClass");
	RegisterClass(&wndclass);

	g_hInstance = hInstance;
	GetClientRect(GetDesktopWindow(), &rect);
	nWidth = 320;
	if (nWidth > rect.right)
		nWidth = rect.right;
	nHeight = 240;
	if (nHeight > rect.bottom)
		nHeight = rect.bottom;
	g_hMain = CreateWindow(wndclass.lpszClassName, _T("GoodPlayer"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, NULL);
	ShowWindow(g_hMain, nCmdShow);
	UpdateWindow(g_hMain);

	if (lpCmdLine[0] != _T('\0'))
	{
		if (lpCmdLine[0] == _T('"'))
		{
			++lpCmdLine;
			lpCmdLine[_tcslen(lpCmdLine) - 1] = _T('\0');
		}
		OpenMedia(lpCmdLine);
	}

	while (GetMessage(&msg, NULL, 0, 0))
   	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#ifdef STACKWALKER
	DeInitAllocCheck();
#endif
	return msg.wParam;
}

int _tmain(int argc, TCHAR *argv[])
{
	int i;
	TCHAR param[256];

	if (argc > 1)
	{
		_tcscpy(param, argv[1]);
		for (i = 2; i < argc; i++)
		{
			_tcscat(param, _T(" "));
			_tcscat(param, argv[i]);
		}
	}
	else
		param[0] = '\0';
	return WinMain(GetModuleHandle(NULL), NULL, param, SW_NORMAL);
}
