#include "stdafx.h"
#include "ResultListWindow.h"


ResultListWindow::ResultListWindow()
	: _hwnd(0), _hwndParent(0), _width(0), _height(0)
{}


ResultListWindow::~ResultListWindow()
{
	if (_hwnd)
		DestroyWindow(_hwnd);
}

LRESULT ResultListWindow::onCreate(HWND hwnd, LPCREATESTRUCT createStruct)
{
	_hwnd = hwnd;
	_hwndParent = createStruct->hwndParent;
}

LRESULT ResultListWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ResultListWindow *classInstance;

	if (uMsg == WM_NCCREATE)
	{
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)((LPCREATESTRUCT)lParam)->lpCreateParams);
	}
	else
	{
		ResultListWindow *classInstance = (ResultListWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		
		switch (uMsg)
		{
		case WM_CREATE:
			return classInstance->onCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_NCDESTROY:
			classInstance->_hwnd = 0;
			classInstance->_hwndParent = 0;
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
