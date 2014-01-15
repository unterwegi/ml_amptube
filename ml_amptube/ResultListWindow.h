#pragma once
#include "HttpHandler.h"

class ResultListWindow
{
public:
	ResultListWindow();
	~ResultListWindow();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	operator HWND(){ return _hwnd; }

	bool createWindow(HWND hwndParent, int controlId,
		int xPos, int yPos, int width, int height);
	void destroyWindow();

	void setResults(const VideoContainer &results);
	VideoContainer getResults() const;

	void clearList();
private:
	typedef std::map<int, HBITMAP>				ThumbnailMap;
	typedef std::map<int, HBITMAP>::value_type	ThumbnailPair;
	static int _instanceCnt;
	static const wchar_t *_wndClassName;
	HWND _hwnd, _hwndParent;
	HFONT _mainFont;
	int _xPos, _yPos;
	int _width, _height;
	int _controlId;

	int _itemHeight;
	VideoContainer _results;
	ThumbnailMap _thumbnails;

	static void registerClass();
	static void unregisterClass();

	void triggerRedraw();

	void fillRect(HDC hdc, LPRECT rect, COLORREF color);
	
	LRESULT onCreate(HWND hwnd, LPCREATESTRUCT createStruct);
	void onWindowPosChanged(LPWINDOWPOS windowPos);
	void onPaint(HDC hdc);
};

