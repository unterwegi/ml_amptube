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
	: _hwnd(0), _hwndParent(0), _mainFont(0), _controlId(0), _hasFocus(false),
	_resultPage(1),	_xPos(0), _yPos(0), _width(0), _height(0),
	_firstItemIdx(0), _selectedItemIdx(INT_MAX)
{
	++_instanceCnt;

	_itemHeight = 130;
	_minItemWidth = 400;
	_thumbnailPadding = 15;
	_thumbnailWidth = 150;
	_thumbnailHeight = _itemHeight - _thumbnailPadding * 2;
	_textStartXPos = _thumbnailWidth + _thumbnailPadding * 2;
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

void ResultListWindow::startSearch(const std::wstring &query)
{
	_resultPage = 1;
	_searchQuery = query;
	if (!_searchQuery.empty())
		doSearch();
}

void ResultListWindow::nextResultPage()
{
	if (!_searchQuery.empty())
	{
		++_resultPage;
		doSearch();
	}
}

void ResultListWindow::prevResultPage()
{
	if (!_searchQuery.empty() && _resultPage > 1)
	{
		--_resultPage;
		doSearch();
	}
}

std::wstring ResultListWindow::getSearchQuery() const
{
	return _searchQuery;
}

int ResultListWindow::getCurrentPage() const
{
	return _resultPage;
}

void ResultListWindow::doSearch()
{
	deleteThumbnails();

	HttpHandler::instance().doSearch(_searchQuery, _resultPage, MaxResults, [&]
		(const VideoContainer &results)
	{
		_firstItemIdx = 0;
		_selectedItemIdx = INT_MAX;
		_results = results;

		HttpHandler::instance().retrieveThumbnails(_results, [=]
			(const std::wstring &videoId, const std::wstring &fileName)
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
			_thumbnails.insert(ThumbnailPair(videoId, 0));
			triggerRedraw();
		});

		setVScrollBarInfo();
	});
}

void ResultListWindow::clearList()
{
	_searchQuery.clear();
	_results.clear();
	_selectedItemIdx = INT_MAX;
	deleteThumbnails();
	setVScrollBarInfo();
}

void ResultListWindow::deleteThumbnails()
{
	for (auto &bitmap : _thumbnails)
	{
		DeleteObject(bitmap.second);
	}
	_thumbnails.clear();
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

ResultListWindow::SizeType ResultListWindow::getItemsPerPage()
{
	return _height / _itemHeight;
}

ResultListWindow::SizeType ResultListWindow::getLastItemIdx()
{
	return std::max(_results.size() - getItemsPerPage(), (SizeType) 0);
}

void ResultListWindow::setVScrollBarInfo()
{
	if (!_hwnd) return;

	SCROLLINFO scrollInfo = { 0 };
	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_ALL;
	scrollInfo.nMin = 0;
	scrollInfo.nMax = _results.size();
	scrollInfo.nPage = getItemsPerPage() + 1;
	scrollInfo.nPos = _firstItemIdx;
	SetScrollInfo(_hwnd, SB_VERT, &scrollInfo, TRUE);
	MLSkinnedScrollWnd_UpdateBars(_hwnd, TRUE);

	triggerRedraw();
}

ResultListWindow::SizeType ResultListWindow::getItemIdxForPosition(POINT position)
{
	SizeType resultItem = INT_MAX;
	RECT itemRect;
	GetClientRect(_hwnd, &itemRect);
	itemRect.bottom = _itemHeight;

	for (SizeType itemIdx = 0; itemIdx < _results.size(); ++itemIdx)
	{
		if (itemIdx >= _firstItemIdx)
		{
			if (PtInRect(&itemRect, position))
			{
				resultItem = itemIdx;
				break;
			}
			else
			{
				itemRect.top = itemRect.bottom;
				itemRect.bottom += _itemHeight;
			}
		}

		if (itemRect.top >= _height)
			break;
	}

	return resultItem;
}

LRESULT ResultListWindow::onCreate(HWND hwnd, const CREATESTRUCT &createStruct)
{
	_mainFont = (HFONT) SendMessage(_hwndParent, WM_GETFONT, 0, 0);
	_hwnd = hwnd;
	setVScrollBarInfo();
	return 0;
}

void ResultListWindow::onWindowPosChanged(const WINDOWPOS &windowPos)
{
	_xPos = windowPos.x;
	_xPos = windowPos.y;
	_width = windowPos.cx;
	_height = windowPos.cy;

	setVScrollBarInfo();
}

void ResultListWindow::onVScroll(WORD scrollRequest, const SCROLLINFO &scrollInfo)
{
	int newFirstItemIdx = _firstItemIdx;
	SizeType itemsPerPage = getItemsPerPage();
	SizeType lastItemIdx = getLastItemIdx();

	switch (scrollRequest)
	{
	case SB_TOP:
		newFirstItemIdx = 0;
		break;
	case SB_PAGEUP:
		newFirstItemIdx -= itemsPerPage;
		break;
	case SB_LINEUP:
		--newFirstItemIdx;
		break;
	case SB_THUMBTRACK:
		newFirstItemIdx = scrollInfo.nTrackPos;
		break;
	case SB_LINEDOWN:
		++newFirstItemIdx;
		break;
	case SB_PAGEDOWN:
		newFirstItemIdx += itemsPerPage;
		break;
	case SB_BOTTOM:
		newFirstItemIdx = lastItemIdx;
		break;
	}

	if (newFirstItemIdx > (int) lastItemIdx)
		newFirstItemIdx = lastItemIdx;

	if (newFirstItemIdx < 0)
		newFirstItemIdx = 0;

	if ((int) _firstItemIdx != newFirstItemIdx)
	{
		_firstItemIdx = newFirstItemIdx;
		setVScrollBarInfo();
	}
}

void ResultListWindow::onMouseWheel(short wheelDelta, WORD keys, POINT &cursorPos)
{
	//wheelDelta is positive if the user scrolls up (wheel rotates away from him)
	if (wheelDelta > 0 && _firstItemIdx > 0)
	{
		--_firstItemIdx;
		setVScrollBarInfo();
	}
	//and it is negative if he scrolls down (wheel rotates towards him)
	else if (wheelDelta < 0 && _firstItemIdx < getLastItemIdx())
	{
		++_firstItemIdx;
		setVScrollBarInfo();
	}
}

void ResultListWindow::onLMouseButtonUp(WORD keys, POINT &cursorPos)
{
	SizeType oldSelection = _selectedItemIdx;
	_selectedItemIdx = getItemIdxForPosition(cursorPos);

	if (SetFocus(_hwnd) == _hwnd && oldSelection != _selectedItemIdx)
		triggerRedraw();
}

void ResultListWindow::onLMouseButtonDblClk(WORD keys, POINT &cursorPos)
{

}

void ResultListWindow::onKeyDown(DWORD vKey)
{
	int newSelectedItemIdx = _selectedItemIdx;
	SizeType itemsPerPage = getItemsPerPage();
	SizeType lastItemIdx = _results.size() - 1;

	switch (vKey)
	{
	case VK_HOME:
		newSelectedItemIdx = 0;
		break;
	case VK_PRIOR:
		if (newSelectedItemIdx != (int) _firstItemIdx)
			newSelectedItemIdx = _firstItemIdx;
		else
			newSelectedItemIdx = _firstItemIdx - itemsPerPage + 1;
		break;
	case VK_UP:
		if (newSelectedItemIdx == INT_MAX)
			newSelectedItemIdx = 0;
		else
			--newSelectedItemIdx;
		break;
	case VK_DOWN:
		if (newSelectedItemIdx == INT_MAX)
			newSelectedItemIdx = 0;
		else
			++newSelectedItemIdx;
		break;
	case VK_NEXT:
		if (newSelectedItemIdx != (int)(_firstItemIdx + itemsPerPage - 1))
			newSelectedItemIdx = _firstItemIdx + itemsPerPage - 1;
		else
			newSelectedItemIdx += itemsPerPage - 1;
		break;
	case VK_END:
		newSelectedItemIdx = lastItemIdx;
		break;
	}

	if (newSelectedItemIdx > (int)lastItemIdx)
		newSelectedItemIdx = lastItemIdx;

	if (newSelectedItemIdx < 0)
		newSelectedItemIdx = 0;

	if ((int)_selectedItemIdx != newSelectedItemIdx)
	{
		_selectedItemIdx = newSelectedItemIdx;

		if (_selectedItemIdx < _firstItemIdx)
			_firstItemIdx = _selectedItemIdx;

		if (_selectedItemIdx > _firstItemIdx + getItemsPerPage() - 1)
			_firstItemIdx = _selectedItemIdx - getItemsPerPage() + 1;

		setVScrollBarInfo();
	}
}

void ResultListWindow::onPaint(HDC hdc, RECT &updateRect)
{
	const ThumbnailMap tempThumbnails = _thumbnails;

	COLORREF bgColor = ml_get_skin_color(WADLG_ITEMBG),
		evenItemFgColor = ml_get_skin_color(WADLG_ITEMFG),
		evenItemBgColor = ml_get_skin_color(WADLG_ITEMBG),
		unevenItemFgColor = ml_get_skin_color(WADLG_ITEMFG2),
		unevenItemBgColor = ml_get_skin_color(WADLG_ITEMBG2),
		activeSelFgColor = ml_get_skin_color(WADLG_SELBAR_FGCOLOR),
		activeSelBgColor = ml_get_skin_color(WADLG_SELBAR_BGCOLOR),
		inactiveSelFgColor = ml_get_skin_color(WADLG_INACT_SELBAR_FGCOLOR),
		inactiveSelBgColor = ml_get_skin_color(WADLG_INACT_SELBAR_BGCOLOR),
		lFgColor, lBgColor;

	HPEN thumbBorderPen = CreatePen(PS_SOLID, 1, ml_get_skin_color(WADLG_WNDFG));

	int saveState = SaveDC(hdc);

	SelectObject(hdc, thumbBorderPen);
	SelectObject(hdc, GetStockObject(NULL_BRUSH));

	RECT rect;
	SetRect(&rect, 0, 0, 5, _height);
	fillRect(hdc, &rect, bgColor);

	RECT itemRect;
	GetClientRect(_hwnd, &itemRect);
	itemRect.left = 5;
	itemRect.bottom = _itemHeight;
	SetBkMode(hdc, TRANSPARENT);
	if (_mainFont)
		SelectObject(hdc, _mainFont);

	SizeType itemIdx = 0;
	for (const auto &video : _results)
	{
		if (itemIdx >= _firstItemIdx)
		{
			if (itemIdx == _selectedItemIdx)
			{
				lFgColor = _hasFocus ? activeSelFgColor : inactiveSelFgColor;
				lBgColor = _hasFocus ? activeSelBgColor : inactiveSelBgColor;
			}
			else
			{
				lFgColor = (itemIdx % 2 == 0) ? evenItemFgColor : unevenItemFgColor;
				lBgColor = (itemIdx % 2 == 0) ? evenItemBgColor : unevenItemBgColor;
			}

			SetTextColor(hdc, lFgColor);
			fillRect(hdc, &itemRect, lBgColor);

			RECT thumbRect = itemRect;
			thumbRect.left += _thumbnailPadding;
			thumbRect.top += _thumbnailPadding;
			thumbRect.right = thumbRect.left + _thumbnailWidth;
			thumbRect.bottom = thumbRect.top + _thumbnailHeight;

			Rectangle(hdc, thumbRect.left, thumbRect.top, thumbRect.right, thumbRect.bottom);

			auto thumbnail = tempThumbnails.find(video.getId());

			if (thumbnail != tempThumbnails.cend())
			{
				fillRect(hdc, &thumbRect, RGB(255, 255, 255));
			}

			RECT textRect = itemRect;
			textRect.left = _textStartXPos;
			DrawText(hdc, video.getTitle().c_str(), video.getTitle().length(), &textRect,
				DT_NOCLIP | DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
		
			itemRect.top = itemRect.bottom;
			itemRect.bottom += _itemHeight;
		}

		//only draw items which are in the visible area of the window
		if (itemRect.top >= _height)
			break;

		++itemIdx;
	}

	if (itemRect.top < _height)
	{
		itemRect.bottom = _height;
		fillRect(hdc, &itemRect, bgColor);
	}

	RestoreDC(hdc, saveState);

	DeleteObject(thumbBorderPen);
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
			return classInstance->onCreate(hwnd, *(LPCREATESTRUCT)lParam);

		case WM_WINDOWPOSCHANGED:
			classInstance->onWindowPosChanged(*(LPWINDOWPOS)lParam);
			return 0;

		case WM_VSCROLL:
		{
			SCROLLINFO scrollInfo;
			scrollInfo.cbSize = sizeof(scrollInfo);
			scrollInfo.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &scrollInfo);
			classInstance->onVScroll(LOWORD(wParam), scrollInfo);
			return 0;
		}

		case WM_MOUSEWHEEL:
		{
			POINT cursorPos;
			cursorPos.x = GET_X_LPARAM(lParam);
			cursorPos.y = GET_Y_LPARAM(lParam);
			classInstance->onMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam),
				GET_KEYSTATE_WPARAM(wParam), cursorPos);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			POINT cursorPos;
			cursorPos.x = GET_X_LPARAM(lParam);
			cursorPos.y = GET_Y_LPARAM(lParam);
			classInstance->onLMouseButtonUp(GET_KEYSTATE_WPARAM(wParam), cursorPos);
			return 0;
		}

		case WM_LBUTTONDBLCLK:
		{
			POINT cursorPos;
			cursorPos.x = GET_X_LPARAM(lParam);
			cursorPos.y = GET_Y_LPARAM(lParam);
			classInstance->onLMouseButtonDblClk(GET_KEYSTATE_WPARAM(wParam), cursorPos);
			return 0;
		}

		case WM_KEYDOWN:
			classInstance->onKeyDown(wParam);
			return 0;

		case WM_GETDLGCODE:
			/*This message is sent by the dialog navigation logic to this window
			if it has the keyboard focus.
			Normally, messages from pressed arrow keys are processed by the dialog
			navigation logic and therefor no arrow key messages are sent to the window itself.
			But we can return DLGC_WANTARROWS here and this tells the dialog navigation logic
			that we want to handle pressed arrow keys for ourselves. */
			return DLGC_WANTARROWS;

		case WM_SETFOCUS:
			classInstance->_hasFocus = true;

			if ((HWND)wParam != hwnd)
				classInstance->triggerRedraw();
			return 0;

		case WM_KILLFOCUS:
			classInstance->_hasFocus = false;

			if ((HWND)wParam != hwnd)
				classInstance->triggerRedraw();
			return 0;

		case WM_SETFONT:
			classInstance->_mainFont = (HFONT)wParam;
			if (LOWORD(lParam) != 0)
				classInstance->triggerRedraw();
			return 0;

		case WM_GETFONT:
			return (LRESULT) classInstance->_mainFont;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			classInstance->onPaint(hdc, ps.rcPaint);
			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_PRINTCLIENT:
		{
			HDC hdc = (HDC)wParam;
			RECT rect;
			GetClientRect(hwnd, &rect);
			classInstance->onPaint(hdc, rect);
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
