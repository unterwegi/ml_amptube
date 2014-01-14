#pragma once
class ResultListWindow
{
public:
	ResultListWindow();
	~ResultListWindow();

	static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND _hwnd, _hwndParent;
	int _width, _height;

	LRESULT onCreate(HWND hwnd, LPCREATESTRUCT createStruct);
};

