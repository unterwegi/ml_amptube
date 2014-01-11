#pragma once

///<summary>
/// Manages all properties of the plugin, which should be serialized. Uses the singleton pattern.</summary>
///<remarks>
/// Complete list of properties: <br />
///  - "cachePath": Path where the downloaded videos are cached</remarks>
class PluginProperties
{
public:
	///<summary>
	/// Returns a reference to the static class instance.</summary>
	///<returns>
	/// A reference to the static class instance.</returns>
	static PluginProperties &instance()
	{
		static PluginProperties _instance;
		return _instance;
	}

	~PluginProperties(){}

	///<summary>
	/// Sets the value of the given property, and serializes it.</summary>
	///<remarks>
	/// If there is no property with the given name, the method does nothing.</remarks>
	///<param name="name"> The name of the property.</param>
	///<param name="value"> The new value of the property.</param>
	void setProperty(std::wstring name, std::wstring value);

	///<summary>
	/// Gets the value of the given property.</summary>
	///<param name="name"> The name of the property.</param>
	///<returns>
	/// The value of the property, if the property exists, or an empty string otherwise.</returns>
	std::wstring getProperty(std::wstring name) const;

	///<summary>
	/// Checks if a property exists.</summary>
	///<param name="name"> The name of the property.</param>
	///<returns>
	/// True, if the property exists, false otherwise.</returns>
	bool hasProperty(std::wstring name) const;
private:
	typedef std::map<std::wstring, std::wstring>	PropertyMap;
	typedef PropertyMap::value_type					PropertyPair;

	///<summary>
	/// The name of the INI section, which is used for the serialized properties.</summary>
	std::string _iniSection;

	///<summary>
	/// A map containing all properties with their current values</summary>
	PropertyMap _properties;

	///<summary>
	/// Private constructor. Needed for proper singleton.</summary>
	///<remarks>
	/// The constructor sets all default values for the properties.</remarks>
	PluginProperties();
	///<summary>
	/// Private copy constructor. Needed for proper singleton.</summary>
	PluginProperties(const PluginProperties&) {}
	///<summary>
	/// Private assignment operator. Needed for proper singleton.</summary>
	PluginProperties& operator=(const PluginProperties&) {}

	///<summary>
	/// Loads all properties from the serialization INI file</summary>
	void loadProperties();
};

