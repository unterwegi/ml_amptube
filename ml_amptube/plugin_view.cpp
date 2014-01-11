#include "stdafx.h"
#include "plugin_view.h"
#include "ml_amptube.h"

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

static void FillResultList(const VideoContainer &searchResults)
{
	if (!listWnd) return;

	LVITEM listItem = { 0 };
	SendMessage(listWnd, LVM_DELETEALLITEMS, 0, 0);

	listItem.mask = LVIF_TEXT;
	listItem.iItem = 0;
	for (const auto &item : searchResults)
	{
		listItem.iSubItem = 0;
		std::wstring buf = item.getTitle();
		listItem.pszText = const_cast<wchar_t*>(buf.c_str());
		SendMessage(listWnd, LVM_INSERTITEM, 0, (LPARAM)&listItem);

		listItem.iSubItem = 1;
		buf = item.getUploader();
		listItem.pszText = const_cast<wchar_t*>(buf.c_str());
		SendMessage(listWnd, LVM_SETITEM, 0, (LPARAM)&listItem);

		listItem.iSubItem = 2;
		buf = item.getId();
		listItem.pszText = const_cast<wchar_t*>(buf.c_str());
		SendMessage(listWnd, LVM_SETITEM, 0, (LPARAM)&listItem);
		listItem.iItem += 1;
	}
}

static BOOL amptube_View_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	/* gen_ml has some helper functions to deal with skinned dialogs,
	we're going to grab their function pointers.
	for definition of magic numbers, see gen_ml/ml.h	 */
	if (!ml_childresize_init)
	{
		/* skinning helper functions */
		ml_hook_dialog_msg = (HookDialogFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)2, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_draw = (DrawFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)3, ML_IPC_SKIN_WADLG_GETFUNC);

		/* resizing helper functions */
		ml_childresize_init = (ChildResizeFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)32, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_childresize_resize = (ChildResizeFunc)SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)33, ML_IPC_SKIN_WADLG_GETFUNC);
	}

	listWnd = GetDlgItem(hwnd, IDC_LIST);

	/* add listview columns */
	LVCOLUMN lvc = { 0, };
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = (LPTSTR)L"Title";
	lvc.cx = 250;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)0, (LPARAM)&lvc);

	lvc.pszText = (LPTSTR)L"Uploader";
	lvc.cx = 150;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)1, (LPARAM)&lvc);

	lvc.pszText = (LPTSTR)L"ID";
	lvc.cx = 80;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)2, (LPARAM)&lvc);

	/* skin dialog */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwnd;
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	/* skin listview */
	sw.hwndToSkin = listWnd;
	sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
	sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
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
	wchar_t buffer[512];

	switch (ctrlId)
	{
	case IDC_CLEAR:
		SendMessage(GetDlgItem(hwnd, IDC_SEARCH), WM_GETTEXT, 512, (LPARAM)&buffer);
		HttpHandler::instance().doSearch(buffer, 1, 10, FillResultList);
		return TRUE;
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
		//TODO: Remove all .mp4 and .flv files in the current cache path
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