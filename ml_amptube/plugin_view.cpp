#include "stdafx.h"
#include "plugin_view.h"
#include "ml_amptube.h"

GetSkinColorFunc ml_get_skin_color = 0;
HookDialogFunc ml_hook_dialog_msg = 0;
DrawFunc ml_draw = 0;
ChildResizeFunc ml_childresize_init = 0, ml_childresize_resize = 0;
HFONT mainFont = 0;

static ChildWndResizeItem plugin_view_rlist[] =
{
	{ IDC_SEARCH, 0x0010 },
	{ IDC_LIST, 0x0011 },
};

INT_PTR CreatePluginView(INT_PTR treeItem, HWND parent)
{
	if (treeItem == PluginTreeItem)
	{
		return (INT_PTR)CreateDialog(Plugin.hDllInstance, MAKEINTRESOURCE(IDD_VIEW_AMPTUBE), parent, MainPluginViewProc);
	}
	else
	{
		return 0;
	}
}

INT_PTR CreatePluginConfigDialog(HWND parent)
{
	return (INT_PTR)CreateDialog(Plugin.hDllInstance, MAKEINTRESOURCE(IDD_CONFIG_AMPTUBE), parent, ConfigPluginProc);
}

static BOOL amptube_View_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	/* gen_ml has some helper functions to deal with skinned dialogs,
	we're going to grab their function pointers.
	for definition of magic numbers, see gen_ml/ml.h	 */
	if (!ml_get_skin_color)
	{
		/* skinning helper functions */
		ml_get_skin_color = (GetSkinColorFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)1, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_hook_dialog_msg = (HookDialogFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)2, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_draw = (DrawFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)3, ML_IPC_SKIN_WADLG_GETFUNC);
		/* resizing helper functions */
		ml_childresize_init = (ChildResizeFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)32, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_childresize_resize = (ChildResizeFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)33, ML_IPC_SKIN_WADLG_GETFUNC);
		mainFont = (HFONT)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)66, ML_IPC_SKIN_WADLG_GETFUNC);
	}

	RECT wndRect, btnRect;
	GetClientRect(hwnd, &wndRect);
	GetClientRect(GetDlgItem(hwnd, IDC_CLEAR), &btnRect);
	/* Magic number offsets are determined by try & error to match winamp positions as closely as possible */
	int xPos = 1,
		yPos = btnRect.bottom + 6,
		width = wndRect.right - xPos - 3,
		height = wndRect.bottom - yPos - btnRect.bottom - 5;

	resultList.createWindow(hwnd, IDC_LIST, xPos, yPos, width, height);

	/* skin dialog */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwnd;
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	/* skin resultList */
	sw.skinType = SKINNEDWND_TYPE_SCROLLWND;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = resultList;
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	/* skin button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_CLEAR);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	/* skin static */
	sw.skinType = SKINNEDWND_TYPE_STATIC;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_STATIC);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	/* skin edit */
	sw.skinType = SKINNEDWND_TYPE_EDIT;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_SEARCH);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	ml_childresize_init(hwnd, plugin_view_rlist, sizeof(plugin_view_rlist) / sizeof(plugin_view_rlist[0]));

	return FALSE;
}

static BOOL amptube_View_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		RECT parentRect, buttonRect;
		ml_childresize_resize(hwnd, plugin_view_rlist, sizeof(plugin_view_rlist) / sizeof(plugin_view_rlist[0]));
		HWND buttonHwnd = GetDlgItem(hwnd, IDC_CLEAR);
		GetWindowRect(buttonHwnd, &buttonRect);
		MapWindowPoints(buttonHwnd, hwnd, (LPPOINT) &buttonRect, 2);
		int buttonWidth = buttonRect.right - buttonRect.left;
		GetClientRect(hwnd, &parentRect);
		SetWindowPos(buttonHwnd, 0,
			parentRect.right - buttonWidth - 2, 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return FALSE;
}

static BOOL amptube_View_OnCommand(HWND hwnd, HWND ctrlHwnd, WORD ctrlId, WORD code)
{
	switch (ctrlId)
	{
	case IDC_SEARCH:
		if (code == EN_CHANGE)
		{
			//This KillTimer/SetTimer combo + the handler in amptube_View_OnTimer 
			//causes that a search is only triggered if the user does not press a key
			//for at least editTimerElapse milliseconds (1 second)
			//This prevents a search spam which could result in a block from Google for a certain amount of time
			KillTimer(hwnd, editTimerId);
			SetTimer(hwnd, editTimerId, editTimerElapse, NULL);
			return TRUE;
		}
		break;

	case IDC_CLEAR:
		SetWindowText(GetDlgItem(hwnd, IDC_SEARCH), L"");
		resultList.clearList();
		return TRUE;
	}
	return 0;
}

static BOOL amptube_View_OnTimer(HWND hwnd, UINT_PTR timerId)
{
	wchar_t buffer[512];

	switch (timerId)
	{
	case editTimerId:
		KillTimer(hwnd, editTimerId);
		GetWindowText(GetDlgItem(hwnd, IDC_SEARCH), buffer, 512);
		HttpHandler::instance().doSearch(buffer, 1, 10, [&]
			(const VideoContainer &results)
		{
			resultList.setResults(results);
		});
		break;
	}

	return 0;
}

static BOOL amptube_View_OnDestroy(HWND hwnd)
{
	return FALSE;
}

INT_PTR CALLBACK MainPluginViewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* first, ask the dialog skinning system if it wants to do anything with the message
	the function pointer gets set during WM_INITDIALOG so might be NULL for the first few messages */
	if (ml_hook_dialog_msg)
	{
		INT_PTR a = ml_hook_dialog_msg(hwndDlg, uMsg, wParam, lParam);
		if (a)
			return a;
	}

	switch (uMsg)
	{
	case WM_INITDIALOG:
		return amptube_View_OnInitDialog(hwndDlg, (HWND)wParam, lParam);

	case WM_SIZE:
		return amptube_View_OnSize(hwndDlg, (UINT)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	case WM_COMMAND:
		return amptube_View_OnCommand(hwndDlg, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));

	case WM_TIMER:
		return amptube_View_OnTimer(hwndDlg, (UINT_PTR) wParam);

	case WM_PAINT:
	{
		 int tab[] = { IDC_LIST | DCW_SUNKENBORDER };
		 ml_draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
		 return 0;
	}

	case WM_DESTROY:
		return amptube_View_OnDestroy(hwndDlg);
	}
	return FALSE;
}

static BOOL amptube_Config_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	HWND editWnd = GetDlgItem(hwnd, IDC_CACHE_PATH);
	SendMessage(editWnd, EM_SETLIMITTEXT, MAX_PATH, 0);
	SetWindowText(editWnd,
		PluginProperties::instance().getProperty(L"cachePath").c_str());
	return FALSE;
}

static BOOL amptube_Config_OnCommand(HWND hwnd, HWND ctrlHwnd, WORD ctrlId, WORD code)
{
	wchar_t buffer[MAX_PATH];

	switch (ctrlId)
	{
	case IDC_BROWSE_PATH:
		//TODO: Show a Browse for folder dialog and set it as the content of the cache path edit field
		return TRUE;

	case IDC_CLEAR_CACHE:
		//TODO: Remove all .mp4, .jpg and .flv files in the current cache path
		return TRUE;

	case IDOK:
		GetWindowText(GetDlgItem(hwnd, IDC_CACHE_PATH), buffer, MAX_PATH);
		PluginProperties::instance().setProperty(L"cachePath", buffer);
		DestroyWindow(hwnd);
		return TRUE;

	case IDCANCEL:
		DestroyWindow(hwnd);
		return TRUE;
	}

	return 0;
}

static BOOL amptube_Config_OnDestroy(HWND hwnd)
{
	return FALSE;
}

INT_PTR CALLBACK ConfigPluginProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return amptube_Config_OnInitDialog(hwndDlg, (HWND)wParam, lParam);

	case WM_COMMAND:
		return amptube_Config_OnCommand(hwndDlg, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));

	case WM_DESTROY:
		return amptube_Config_OnDestroy(hwndDlg);
	}

	return FALSE;
}