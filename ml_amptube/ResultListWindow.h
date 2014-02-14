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

	void playSelectedItems();
private:
	struct ThumbnailData
	{
		HBITMAP handle;
		unsigned char *bitmapData;
		BITMAPINFO bitmapInfo;
		int width;
		int height;
	};

	typedef std::map<std::wstring, ThumbnailData>	ThumbnailMap;
	typedef ThumbnailMap::value_type				ThumbnailPair;
	typedef VideoContainer::size_type				SizeType;

	static int _instanceCnt;
	static const wchar_t *_wndClassName;
	HWND _hwnd, _hwndParent;
	HFONT _mainFont;
	int _controlId;
	bool _hasFocus;

	HDC _bufferDc;
	HBITMAP _bufferBitmap;
	int _bufferDcInitState;

	std::wstring _searchQuery;
	int _resultPage;
	
	int _xPos, _yPos;
	int _width, _height;
	SizeType _firstItemIdx, _selectedItemIdx;

	int _itemHeight, _minItemWidth;
	int _thumbnailPadding;
	int _thumbnailWidth, _thumbnailHeight;
	int _textStartXPos;

	VideoContainer _results;
	ThumbnailMap _thumbnails;

	static void registerClass();
	static void unregisterClass();

	void deleteThumbnails();
	void doSearch();
	void triggerRedraw();

	void fillRect(HDC hdc, LPRECT rect, COLORREF color);

	void destroyBufferDc();
	void rebuildBufferDc();

	SizeType getItemsPerPage();
	SizeType getLastItemIdx();
	void setVScrollBarInfo();

	RECT getItemRect(SizeType itemIdx);
	SizeType getItemIdxForPosition(POINT position);
	
	LRESULT onCreate(HWND hwnd, const CREATESTRUCT &createStruct);
	void onWindowPosChanged(const WINDOWPOS &windowPos);
	void onVScroll(WORD scrollRequest, const SCROLLINFO &scrollInfo);
	void onMouseWheel(short wheelDelta, WORD keys, POINT &cursorPos);
	void onLMouseButtonUp(WORD keys, POINT &cursorPos);
	void onLMouseButtonDblClk(WORD keys, POINT &cursorPos);
	void onKeyDown(DWORD vKey);
	void onPaint(HDC hdc);
	void onNcDestroy();
};

