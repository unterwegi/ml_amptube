#include "stdafx.h"
#include "PluginProperties.h"
#include "ml_amptube.h"

PluginProperties::PluginProperties()
{
	_iniSection = "ml_amptube";
	waServiceFactory *factory = ServiceManager->service_getServiceByGuid(applicationApiServiceGuid);
	if (factory)
	{
		api_application *app_api = (api_application *) factory->getInterface();
		std::wstring cachePath = std::wstring(app_api->path_getUserSettingsPath()) + L"\\Plugins\\ml_amptube_cache";
		factory->Release();
		_properties.insert(PropertyPair(L"cachePath", cachePath));
	}
	
	loadProperties();
}

void PluginProperties::loadProperties()
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	char *iniPath = (char *) SendMessage(Plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIFILE);

	for (auto &prop : _properties)
	{
		std::string propNameUtf8 = conv.to_bytes(prop.first);
		char propValueUtf8[512];

		GetPrivateProfileStringA(_iniSection.c_str(), propNameUtf8.c_str(), NULL, propValueUtf8, 512, iniPath);

		if (strlen(propValueUtf8) > 0)
			prop.second = conv.from_bytes(propValueUtf8);
		else
			setProperty(prop.first, prop.second);
	}
}

void PluginProperties::setProperty(std::wstring name, std::wstring value)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	char *iniPath = (char *)SendMessage(Plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIFILE);
	auto prop = _properties.find(name);

	if (prop != _properties.end())
	{
		prop->second = value;
		std::string propNameUtf8 = conv.to_bytes(prop->first);
		std::string propValueUtf8 = conv.to_bytes(value);
		WritePrivateProfileStringA(_iniSection.c_str(), propNameUtf8.c_str(), propValueUtf8.c_str(), iniPath);
	}
}

std::wstring PluginProperties::getProperty(std::wstring name) const
{
	std::wstring propVal;
	auto prop = _properties.find(name);

	if (prop != _properties.end())
		propVal =  prop->second;

	return propVal;
}

bool PluginProperties::hasProperty(std::wstring name) const
{
	return (_properties.find(name) != _properties.end());
}

