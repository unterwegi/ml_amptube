#pragma once

#include "VideoFormatParseException.h"

class VideoFormatExtractor
{
public:
	class FormatDescription
	{
	public:
		FormatDescription(std::wstring containerName, std::wstring resolution, bool dashEnabled)
			: _containerName(containerName), _resolution(resolution), _dashEnabled(dashEnabled) {}

		std::wstring getContainerName(){ return _containerName; }
		std::wstring getResolution(){ return _resolution; }
		bool isDashEnabled(){ return _dashEnabled; }

		std::wstring toString()
		{
			return _containerName + L" " + _resolution + L" " +
				(_dashEnabled ? L"DASH" : L"Non-DASH");
		}
	private:
		std::wstring _containerName;
		std::wstring _resolution;
		bool _dashEnabled;
	};

	typedef std::map<int, FormatDescription>	FormatDescriptionMap;
	typedef FormatDescriptionMap::value_type	FormatDescriptionPair;

	typedef std::map<int, std::wstring>	VideoFormatMap;
	typedef VideoFormatMap::value_type	VideoFormatPair;

	///<summary>
	/// Returns a reference to the static class instance.</summary>
	///<returns>
	/// A reference to the static class instance.</returns>
	static VideoFormatExtractor &instance()
	{
		static VideoFormatExtractor _instance;
		return _instance;
	}

	~VideoFormatExtractor(){}

	FormatDescriptionMap getFormatDescriptionMap() const { return _formatDescriptionMap;  }

	VideoFormatMap getVideoFormatMap(const std::wstring &videoId) const;
	
private:
	static FormatDescriptionMap _formatDescriptionMap;
	static std::wstring _watchUri;

	///<summary>
	/// Private constructor. Needed for proper singleton.</summary>
	///<remarks>
	/// The constructor sets all default values for the properties.</remarks>
	VideoFormatExtractor(){}
	///<summary>
	/// Private copy constructor. Needed for proper singleton.</summary>
	VideoFormatExtractor(const VideoFormatExtractor&) {}
	///<summary>
	/// Private assignment operator. Needed for proper singleton.</summary>
	VideoFormatExtractor& operator=(const VideoFormatExtractor&) {}
};

