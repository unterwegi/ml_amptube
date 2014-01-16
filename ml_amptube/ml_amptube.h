#pragma once
#include "resource.h"
#include "plugin_view.h"

int Init();
void Quit();
INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

std::wstring GetLocalString(UINT id);

extern LCID StringLocale;
extern winampMediaLibraryPlugin Plugin;
extern INT_PTR PluginTreeItem;
extern api_service *ServiceManager;