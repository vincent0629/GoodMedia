#include "MessageWindow.h"
#include "platform.h"

CMessageWindow::CMessageWindow()
{
	m_hWnd = CreateWindow(_T("STATIC"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
	m_bDestroy = true;
	Init();
}

CMessageWindow::CMessageWindow(HWND hwnd)
{
	m_hWnd = hwnd;
	m_bDestroy = false;
	Init();
}

CMessageWindow::~CMessageWindow()
{
	SetWindowLong(m_hWnd, GWL_WNDPROC, m_WindowLong);
	if (m_bDestroy)
		DestroyWindow(m_hWnd);
}

void CMessageWindow::SetWindowListener(IWindowListener *pListener)
{
	m_pListener = pListener;
}

void CMessageWindow::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void CMessageWindow::Init(void)
{
	SetWindowLong(m_hWnd, GWL_USERDATA, (long)this);
	m_WindowLong = SetWindowLong(m_hWnd, GWL_WNDPROC, (long)WindowProc);
}

LRESULT CALLBACK CMessageWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMessageWindow *pThis;
	LRESULT lResult;

	pThis = (CMessageWindow *)GetWindowLong(hwnd, GWL_USERDATA);
	lResult = pThis->m_pListener->WindowProc(hwnd, uMsg, wParam, lParam);
	return lResult == 0? 0 : CallWindowProc((WNDPROC)pThis->m_WindowLong, hwnd, uMsg, wParam, lParam);
}
