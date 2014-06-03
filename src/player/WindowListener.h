#ifndef _WINDOWLISTENER_H_
#define _WINDOWLISTENER_H_

#include <windows.h>

class IWindowListener
{
public:
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

#endif
