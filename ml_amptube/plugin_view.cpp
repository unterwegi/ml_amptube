#include "stdafx.h"
#include "plugin_view.h"
#include "VideoFormatExtractor.h"
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

	/* skin result list */
	sw.skinType = SKINNEDWND_TYPE_SCROLLWND;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = resultList;
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);
	SetFocus(resultList);

	/* skin static text */
	sw.skinType = SKINNEDWND_TYPE_STATIC;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_STATIC);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);
	SetWindowText(sw.hwndToSkin, GetLocalString(IDS_SEARCH).c_str());

	/* skin search edit */
	sw.skinType = SKINNEDWND_TYPE_EDIT;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_SEARCH);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);
	SetWindowText(sw.hwndToSkin, resultList.getSearchQuery().c_str());
	KillTimer(hwnd, editTimerId);

	/* skin clear button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_CLEAR);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);
	SetWindowText(sw.hwndToSkin, GetLocalString(IDS_CLEAR_SEARCH).c_str());

	/* skin play options button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_ADD_TO_PLAYLIST);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);
	SetWindowText(sw.hwndToSkin, GetLocalString(IDS_ADD_TO_PLAYLIST).c_str());

	/* skin previous results button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_PREV_RESULTS);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	/* skin next results button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_NEXT_RESULTS);
	MLSkinWindow(Plugin.hwndLibraryParent, &sw);

	/* fix the z-ordering for correct dialog keyboard navigation*/
	SetWindowPos(GetDlgItem(hwnd, IDC_SEARCH), Plugin.hwndLibraryParent,
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hwnd, IDC_CLEAR), GetDlgItem(hwnd, IDC_SEARCH),
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(resultList, GetDlgItem(hwnd, IDC_CLEAR),
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hwnd, IDC_ADD_TO_PLAYLIST), resultList,
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hwnd, IDC_PREV_RESULTS), GetDlgItem(hwnd, IDC_ADD_TO_PLAYLIST),
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hwnd, IDC_NEXT_RESULTS), GetDlgItem(hwnd, IDC_PREV_RESULTS),
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	ml_childresize_init(hwnd, plugin_view_rlist, sizeof(plugin_view_rlist) / sizeof(plugin_view_rlist[0]));

	return FALSE;
}

static BOOL amptube_View_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		RECT parentRect, buttonRect;
		ml_childresize_resize(hwnd, plugin_view_rlist, sizeof(plugin_view_rlist) / sizeof(plugin_view_rlist[0]));
		GetClientRect(hwnd, &parentRect);

		/* move the clear button to the right edge*/
		HWND buttonHwnd = GetDlgItem(hwnd, IDC_CLEAR);	
		GetWindowRect(buttonHwnd, &buttonRect);
		MapWindowPoints(0, hwnd, (LPPOINT) &buttonRect, 2);
		int buttonWidth = buttonRect.right - buttonRect.left;
		SetWindowPos(buttonHwnd, 0,
			parentRect.right - buttonWidth - 2, buttonRect.top, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		/* move the play options button to the bottom edge*/
		buttonHwnd = GetDlgItem(hwnd, IDC_ADD_TO_PLAYLIST);
		GetWindowRect(buttonHwnd, &buttonRect);
		MapWindowPoints(0, hwnd, (LPPOINT)&buttonRect, 2);
		int buttonHeight = buttonRect.bottom - buttonRect.top;
		SetWindowPos(buttonHwnd, 0,
			buttonRect.left, parentRect.bottom - buttonHeight, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		/* move the previous results button to the bottom edge*/
		buttonHwnd = GetDlgItem(hwnd, IDC_PREV_RESULTS);
		GetWindowRect(buttonHwnd, &buttonRect);
		MapWindowPoints(0, hwnd, (LPPOINT)&buttonRect, 2);
		buttonHeight = buttonRect.bottom - buttonRect.top;
		SetWindowPos(buttonHwnd, 0,
			buttonRect.left, parentRect.bottom - buttonHeight, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		/* move the next results button to the bottom edge*/
		buttonHwnd = GetDlgItem(hwnd, IDC_NEXT_RESULTS);
		GetWindowRect(buttonHwnd, &buttonRect);
		MapWindowPoints(0, hwnd, (LPPOINT)&buttonRect, 2);
		buttonHeight = buttonRect.bottom - buttonRect.top;
		SetWindowPos(buttonHwnd, 0,
			buttonRect.left, parentRect.bottom - buttonHeight, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
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
			//This prevents a spam of search requests to Google which could result
			//in a block from them for a certain amount of time
			KillTimer(hwnd, editTimerId);
			SetTimer(hwnd, editTimerId, editTimerElapse, NULL);
			return TRUE;
		}
		break;

	case IDC_CLEAR:
		SetWindowText(GetDlgItem(hwnd, IDC_SEARCH), L"");
		resultList.clearList();
		return TRUE;

	case IDC_ADD_TO_PLAYLIST:
		resultList.addSelectedItemsToPlaylist();
		return TRUE;

	case IDC_PREV_RESULTS:
		if (resultList.getCurrentPage() <= 2)
			EnableWindow(GetDlgItem(hwnd, IDC_PREV_RESULTS), FALSE);

		resultList.prevResultPage();
		return TRUE;

	case IDC_NEXT_RESULTS:
		EnableWindow(GetDlgItem(hwnd, IDC_PREV_RESULTS), TRUE);
		resultList.nextResultPage();
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
		EnableWindow(GetDlgItem(hwnd, IDC_PREV_RESULTS), FALSE);
		resultList.startSearch(buffer);
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
	the function pointer gets set during WM_INITDIALOG so it might be NULL for the first few messages */
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

	case WM_SETFONT:
		mainFont = (HFONT)wParam;
		return 0;

	case WM_GETFONT:
		return (INT_PTR) mainFont;

	case WM_DESTROY:
		return amptube_View_OnDestroy(hwndDlg);
	}
	return FALSE;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);

	return 0;
}

static std::wstring SelectFoler(std::wstring initPath)
{
	wchar_t path[MAX_PATH];


	BROWSEINFO bi = { 0 };
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)initPath.c_str();

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);
		CoTaskMemFree(pidl);
		return path;
	}

	return L"";
}

static BOOL amptube_Config_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SendMessage(GetDlgItem(hwnd, IDC_CACHE_PATH), EM_SETLIMITTEXT, MAX_PATH, 0);
	SetWindowText(GetDlgItem(hwnd, IDC_CACHE_PATH),
		PluginProperties::instance().getProperty(L"cachePath").c_str());

	boost::filesystem::path cachePath(PluginProperties::instance().getProperty(L"cachePath"));

	if (boost::filesystem::is_empty(cachePath))
		EnableWindow(GetDlgItem(hwnd, IDC_CLEAR_CACHE), FALSE);

	int selectedItem = std::stoi(PluginProperties::instance().getProperty(L"desiredQuality"));

	for (const auto &entry : VideoFormatExtractor::instance().getVideoQualitiesOrderMap())
	{
		SendMessage(GetDlgItem(hwnd, IDC_DESIRED_QUALITY), CB_ADDSTRING, 0, (LPARAM) entry.second.c_str());
	}

	SendMessage(GetDlgItem(hwnd, IDC_DESIRED_QUALITY), CB_SETCURSEL, selectedItem, 0);

	return FALSE;
}

static BOOL amptube_Config_OnCommand(HWND hwnd, HWND ctrlHwnd, WORD ctrlId, WORD code)
{
	switch (ctrlId)
	{
	case IDC_BROWSE_PATH:
	{
		std::wstring newPath = SelectFoler(PluginProperties::instance().getProperty(L"cachePath"));
		if (!newPath.empty())
			SetWindowText(GetDlgItem(hwnd, IDC_CACHE_PATH), newPath.c_str());

		return TRUE;
	}

	case IDC_CLEAR_CACHE:
	{
		boost::filesystem::path cachePath(PluginProperties::instance().getProperty(L"cachePath"));
		boost::filesystem::directory_iterator end;
		for (boost::filesystem::directory_iterator it(cachePath); it != end; ++it)
		{
			//remove all files and folders in the cache path except files having the extension .incomplete (active downloads)
			if (!(it->path().extension() == ".incomplete"))
				remove_all(it->path());
		}

		EnableWindow(GetDlgItem(hwnd, IDC_CLEAR_CACHE), FALSE);
		return TRUE;
	}

	case IDOK:
	{
		wchar_t buffer[MAX_PATH];
		GetWindowText(GetDlgItem(hwnd, IDC_CACHE_PATH), buffer, MAX_PATH);
		int selectedItem = SendMessage(GetDlgItem(hwnd, IDC_DESIRED_QUALITY), CB_GETCURSEL, 0, 0);
		PluginProperties::instance().setProperty(L"cachePath", buffer);
		PluginProperties::instance().setProperty(L"desiredQuality", std::to_wstring(selectedItem));
		DestroyWindow(hwnd);
		return TRUE;
	}

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