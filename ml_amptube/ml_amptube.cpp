#include "stdafx.h"
#include "ml_amptube.h"

WORD StringLang = MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL);
INT_PTR PluginTreeItem = 0;
api_service *ServiceManager = 0;
api_language *languageManager = 0;
HINSTANCE api_localised_hinstance = 0;
HINSTANCE api_orig_hinstance = 0;

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
	waServiceFactory *factory = ServiceManager->service_getServiceByGuid(languageApiGUID);
	if (factory)
	{
		languageManager = (api_language *)factory->getInterface();
		factory->Release();
	}
	else
		return -1;

	//initialize localization functionality
	WASABI_API_START_LANG(GetModuleHandle(0), WinampLangGUID);

	std::wstring langName = languageManager->GetLanguageIdentifier(LANG_LANG_CODE);

	if (langName == L"de")
		StringLang = MAKELANGID(LANG_GERMAN, SUBLANG_NEUTRAL);

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

std::wstring GetLocalString(UINT id)
{
	std::wstring result = L"";
	DWORD blockId = (id / 16) + 1;
	DWORD stringIdx = id % 16;

	HRSRC resource = FindResourceEx(Plugin.hDllInstance, RT_STRING, MAKEINTRESOURCE(blockId), StringLang);
	if (resource)
	{
		HGLOBAL resBlock = LoadResource(Plugin.hDllInstance, resource);
		if (resBlock)
		{
			const wchar_t *stringTable = (wchar_t *)LockResource(resBlock);
			if (stringTable)
			{
				DWORD blockSize = SizeofResource(Plugin.hDllInstance, resource);
				DWORD blockOffset = 0;
				DWORD actStringIdx = 0;

				//  Search through the section for the appropriate entry.
				//  The first two bytes of each entry is the length of the string
				//  followed by the Unicode string itself. All strings entries 
				//  are stored one after another with no padding.
				while (blockOffset < blockSize)
				{
					if (actStringIdx == stringIdx)
					{
						//  If the string has a size assign it to the result
						if (stringTable[blockOffset] != 0)
						{
							result.assign(&stringTable[blockOffset + 1], stringTable[blockOffset]);
						}
						//  Otherwise the string is empty --> do nothing
						break;
					}

					//  Go to the next string in the table
					blockOffset += stringTable[blockOffset] + 1;
					actStringIdx++;
				}
			}
		}
	}

	return result;
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &Plugin;
}


