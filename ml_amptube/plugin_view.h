#pragma once
#include "resource.h"
#include "HttpHandler.h"

typedef void(*ChildResizeFunc)(HWND, ChildWndResizeItem*, int);
static ChildResizeFunc ml_childresize_init = 0, ml_childresize_resize = 0;

typedef int(*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HookDialogFunc ml_hook_dialog_msg = 0;

typedef void(*DrawFunc)(HWND hwndDlg, int *tab, int tabsize);
static DrawFunc ml_draw = 0;

static HWND listWnd = 0;

INT_PTR CreatePluginView(INT_PTR treeItem, HWND parent);
INT_PTR CreatePluginConfigDialog(HWND parent);

INT_PTR CALLBACK MainPluginViewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConfigPluginProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);