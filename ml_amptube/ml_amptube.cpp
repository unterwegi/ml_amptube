#include "stdafx.h"
#include "ml_amptube.h"

INT_PTR PluginTreeItem = 0;
api_service *ServiceManager = 0;

winampMediaLibraryPlugin Plugin =
{
	MLHDR_VER,
	"AmpTube Media Libary Plugin",
	Init,
	Quit,
	PluginMessageProc,
	0,
	0,
	0,
};

int Init()
{
	//starting point for wasabi, where services are shared
	ServiceManager = (api_service *)SendMessage(Plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	//set up tree item, gen_ml will call amptube_pluginMessageProc if/when the treeview item gets selected
	MLTREEITEMW newTree;
	newTree.size = sizeof(MLTREEITEMW);
	newTree.parentId = 0;
	newTree.title = L"AmpTube";
	newTree.hasChildren = 0;
	newTree.id = 0;
	SendMessage(Plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&newTree, ML_IPC_TREEITEM_ADDW);
	PluginTreeItem = newTree.id;
	return 0; // 0 for success.  returning non-zero will cause gen_ml to abort loading the plugin
}

void Quit()
{
}

INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch (message_type)
	{
	case ML_MSG_TREE_ONCREATEVIEW:     // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us
		return CreatePluginView(param1, (HWND)param2);

	case ML_MSG_CONFIG:
		CreatePluginConfigDialog((HWND)param1);
		return 1;
	}
	return 0;
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &Plugin;
}


