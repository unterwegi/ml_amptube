#pragma once
#include "resource.h"
#include "plugin_view.h"

int Init();
void Quit();
INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

extern winampMediaLibraryPlugin Plugin;
extern INT_PTR PluginTreeItem;
extern api_service *ServiceManager;