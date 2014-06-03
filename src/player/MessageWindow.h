#ifndef _MESSAGEWINDOW_H_
#define _MESSAGEWINDOW_H_

#include "WindowListener.h"

class CMessageWindow
{
public:
	CMessageWindow();
	CMessageWindow(HWND hwnd);
	~CMessageWindow();
	void SetWindowListener(IWindowListener *pListener);
	void PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hWnd;
	bool m_bDestroy;
	long m_WindowLong;
	IWindowListener *m_pListener;

	void Init(void);
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
