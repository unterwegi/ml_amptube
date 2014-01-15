#include "stdafx.h"
#include "ResultListWindow.h"
#include "ml_amptube.h"

int ResultListWindow::_instanceCnt = 0;
const wchar_t *ResultListWindow::_wndClassName = L"AmptubeResultListWindow";

void ResultListWindow::registerClass()
{
	WNDCLASSEX wndClass;

	if (GetClassInfoEx(Plugin.hDllInstance, _wndClassName, &wndClass))
		return;

	wndClass = { 0 };
	wndClass.cbSize = sizeof(wndClass);
	wndClass.lpszClassName = _wndClassName;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = Plugin.hDllInstance;
	wndClass.style = CS_DBLCLKS;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = 0;
	wndClass.hIcon = wndClass.hIconSm = 0;
	wndClass.hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL),
		IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
	wndClass.lpszMenuName = 0;

	RegisterClassEx(&wndClass);
}

void ResultListWindow::unregisterClass()
{
	UnregisterClass(_wndClassName, Plugin.hDllInstance);
}

ResultListWindow::ResultListWindow()
	: _hwnd(0), _hwndParent(0), _mainFont(0),
	_xPos(0), _yPos(0),	_width(0), _height(0), _controlId(0)
{
	++_instanceCnt;
	_itemHeight = 50;
}

ResultListWindow::~ResultListWindow()
{
	clearList();

	if (_hwnd)
		DestroyWindow(_hwnd);

	--_instanceCnt;

	if (_instanceCnt <= 0)
		unregisterClass();
}

bool ResultListWindow::createWindow(HWND hwndParent, int controlId,
	int xPos, int yPos, int width, int height)
{
	_xPos = xPos;
	_yPos = yPos;
	_width = width;
	_height = height;
	_hwndParent = hwndParent;
	_controlId = controlId;

	registerClass();

	return (CreateWindowEx(0,
		_wndClassName, _wndClassName,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
		_xPos, _yPos,
		_width, _height,
		_hwndParent, (HMENU) controlId, Plugin.hDllInstance, this)) != 0;
}

void ResultListWindow::destroyWindow()
{
	if (_hwnd)
		DestroyWindow(_hwnd);
}

void ResultListWindow::setResults(const VideoContainer &results)
{
	_results = results;

	HttpHandler::instance().retrieveThumbnails(_results, [=]
		(int videoIdx, const std::wstring &fileName)
	{
		/*boost::gil::rgb8_image_t image;
		boost::gil::jpeg_read_image("test.jpg", image);

		BITMAPINFO bi24BitInfo; //populate to match rgb8_image_t
		memset(&bi24BitInfo, 0, sizeof(BITMAPINFO));
		bi24BitInfo.bmiHeader.biSize = sizeof(bi24BitInfo.bmiHeader);
		bi24BitInfo.bmiHeader.biBitCount = 24; // rgb 8 bytes for each component(3)
		bi24BitInfo.bmiHeader.biCompression = BI_RGB;// rgb = 3 components
		bi24BitInfo.bmiHeader.biPlanes = 1;
		bi24BitInfo.bmiHeader.biWidth = image.width;
		bi24BitInfo.bmiHeader.biHeight = image.height;

		DIBSECTION d;
		HBITMAP bitmap = CreateDIBSection(NULL, &bi24BitInfo,
		DIB_RGB_COLORS, 0, 0, 0);
		int byteCnt = GetObject(bitmap, sizeof(DIBSECTION), &d);

		memcpy(d.dsBm.bmBits, &(image._view[0]), d.dsBmih.biSizeImage);

		DeleteObject(bitmap);*/
		_thumbnails.insert(ThumbnailPair(videoIdx, 0));
	});
	triggerRedraw();
}

VideoContainer ResultListWindow::getResults() const
{ 
	return _results;
}

void ResultListWindow::clearList()
{
	_results.clear();
	for (auto &bitmap : _thumbnails)
	{
		DeleteObject(bitmap.second);
	}

	_thumbnails.clear();
	triggerRedraw();
}

void ResultListWindow::triggerRedraw()
{
	if (_hwnd)
		InvalidateRect(_hwnd, NULL, FALSE);
}

void ResultListWindow::fillRect(HDC hdc, LPRECT rect, COLORREF color)
{
	int oldMode = SetBkMode(hdc, OPAQUE);
	COLORREF oldColor = SetBkColor(hdc, color);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rect, 0, 0, 0);
	SetBkMode(hdc, oldMode);
	SetBkColor(hdc, oldColor);
}

LRESULT ResultListWindow::onCreate(HWND hwnd, LPCREATESTRUCT createStruct)
{
	_hwnd = hwnd;
	return 0;
}

void ResultListWindow::onWindowPosChanged(LPWINDOWPOS windowPos)
{
	_xPos = windowPos->x;
	_xPos = windowPos->y;
	_width = windowPos->cx;
	_height = windowPos->cy;
}

void ResultListWindow::onPaint(HDC hdc)
{
	COLORREF bgColor = ml_get_skin_color(WADLG_ITEMBG),
		evenItemFgColor = ml_get_skin_color(WADLG_ITEMFG),
		unevenItemFgColor = ml_get_skin_color(WADLG_ITEMFG2),
		evenItemBgColor = ml_get_skin_color(WADLG_ITEMBG),
		unevenItemBgColor = ml_get_skin_color(WADLG_ITEMBG2);

	RECT rect;
	SetRect(&rect, 0, 0, 5, _height);
	fillRect(hdc, &rect, bgColor);

	RECT itemRect;
	GetClientRect(_hwnd, &itemRect);
	itemRect.left = 5;
	itemRect.bottom = _itemHeight;
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, mainFont);

	int itemIdx = 0;
	for (const auto &video : _results)
	{
		SetTextColor(hdc, (itemIdx % 2 == 0 ? evenItemFgColor : unevenItemFgColor));
		fillRect(hdc, &itemRect, (itemIdx % 2 == 0 ? evenItemBgColor : unevenItemBgColor));

		DrawText(hdc, video.getTitle().c_str(), video.getTitle().length(), &itemRect,
			DT_NOCLIP | DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);

		itemRect.top = itemRect.bottom;
		itemRect.bottom += _itemHeight;
		++itemIdx;
	}

	if (itemRect.bottom < _height)
	{
		itemRect.bottom = _height;
		fillRect(hdc, &itemRect, bgColor);
	}
}

LRESULT CALLBACK ResultListWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

		case WM_WINDOWPOSCHANGED:
			classInstance->onWindowPosChanged((LPWINDOWPOS)lParam);
			return 0;

		case WM_SETFONT:
			if (wParam)
				classInstance->_mainFont = (HFONT)wParam;

			if (LOWORD(lParam) != 0)
				InvalidateRect(hwnd, NULL, FALSE);
			return 0;

		case WM_GETFONT:
			return (LRESULT) classInstance->_mainFont;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			int saveState = SaveDC(hdc);
			classInstance->onPaint(hdc);
			RestoreDC(hdc, saveState);
			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_PRINTCLIENT:
		{
			HDC hdc = (HDC)wParam;
			int saveState = SaveDC(hdc);
			classInstance->onPaint(hdc);
			RestoreDC(hdc, saveState);
			return 0;
		}

		case WM_ERASEBKGND:
			return 1;

		case WM_NCDESTROY:
			classInstance->_hwnd = 0;
			classInstance->_hwndParent = 0;
			classInstance->_xPos = 0;
			classInstance->_yPos = 0;
			classInstance->_width = 0;
			classInstance->_height = 0;
			classInstance->_controlId = 0;
			break;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
