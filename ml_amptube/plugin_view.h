#pragma once
#include "resource.h"
#include "HttpHandler.h"
#include "ResultListWindow.h"

typedef int(*GetSkinColorFunc)(int idx);
extern GetSkinColorFunc ml_get_skin_color;

typedef int(*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern HookDialogFunc ml_hook_dialog_msg;

typedef void(*DrawFunc)(HWND hwndDlg, int *tab, int tabsize);
extern DrawFunc ml_draw;

typedef void(*ChildResizeFunc)(HWND, ChildWndResizeItem*, int);
extern ChildResizeFunc ml_childresize_init, ml_childresize_resize;

extern HFONT mainFont;

static const UINT_PTR editTimerId = 1;
static UINT editTimerElapse = 1000;

static ResultListWindow resultList;

INT_PTR CreatePluginView(INT_PTR treeItem, HWND parent);
INT_PTR CreatePluginConfigDialog(HWND parent);

INT_PTR CALLBACK MainPluginViewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConfigPluginProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);