#pragma once
#include "HttpHandler.h"

class ResultListWindow
{
public:
	const static int MaxResults = 10;

	ResultListWindow();
	~ResultListWindow();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	operator HWND(){ return _hwnd; }

	bool createWindow(HWND hwndParent, int controlId,
		int xPos, int yPos, int width, int height);
	void destroyWindow();

	void startSearch(const std::wstring &query);
	void nextResultPage();
	void prevResultPage();

	std::wstring getSearchQuery() const;
	int getCurrentPage() const;
	void clearList();
private:
	typedef std::map<int, HBITMAP>				ThumbnailMap;
	typedef std::map<int, HBITMAP>::value_type	ThumbnailPair;
	static int _instanceCnt;
	static const wchar_t *_wndClassName;
	HWND _hwnd, _hwndParent;
	HFONT _mainFont;
	int _controlId;

	std::wstring _searchQuery;
	int _resultPage;
	
	int _xPos, _yPos;
	int _width, _height;
	int _scrollDrawXOffset, _scrollDrawYOffset;

	int _itemHeight, _minItemWidth;
	int _thumbnailPadding;
	int _thumbnailWidth, _thumbnailHeight;
	int _textStartXPos, _textStartYPos;

	VideoContainer _results;
	ThumbnailMap _thumbnails;

	static void registerClass();
	static void unregisterClass();

	void doSearch();
	void triggerRedraw();

	void fillRect(HDC hdc, LPRECT rect, COLORREF color);
	
	LRESULT onCreate(HWND hwnd, LPCREATESTRUCT createStruct);
	void onWindowPosChanged(LPWINDOWPOS windowPos);
	void onPaint(HDC hdc);
};

