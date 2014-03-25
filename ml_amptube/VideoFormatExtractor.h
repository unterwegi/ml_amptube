#pragma once

#include "VideoFormatParseException.h"
#include "VideoDescription.h"

class VideoFormatExtractor
{
public:
	typedef std::map<int, std::wstring>			VideoQualityOrderMap;

	class FormatDescription
	{
	public:
		FormatDescription(std::wstring containerName, int qualityId, bool usesDash)
			: _containerName(containerName), _qualityId(qualityId), _usesDash(usesDash) {}

		std::wstring getContainerName(){ return _containerName; }
		int getQualityId(){ return _qualityId; }
		bool isDash(){ return _usesDash; }

		std::wstring toString(const VideoQualityOrderMap &videoQualities)
		{
			return _containerName + L" " + videoQualities.find(_qualityId)->second + L" " +
				(_usesDash ? L"DASH" : L"Non-DASH");
		}
	private:
		std::wstring _containerName;
		int _qualityId;
		bool _usesDash;
	};

	typedef std::map<int, FormatDescription>	FormatDescriptionMap;	

	typedef std::map<int, std::wstring>			VideoFormatMap;
	typedef VideoFormatMap::value_type			VideoFormatPair;

	///<summary>
	/// Returns a reference to the static class instance.</summary>
	///<returns>
	/// A reference to the static class instance.</returns>
	static VideoFormatExtractor &instance()
	{
		static VideoFormatExtractor _instance;
		return _instance;
	}

	VideoQualityOrderMap getVideoQualitiesOrderMap() { return _videoQualityOrderMap; }
	int getDefaultDesiredQuality() { return _defaultDesiredQuality; }
	FormatDescriptionMap getFormatDescriptionMap() { return _formatDescriptionMap; }

	~VideoFormatExtractor(){}

	bool startDownload(const VideoDescription &video, 
		std::function<void(std::wstring videoId, int progress, bool finished)> progressChanged) const;
private:
	static VideoQualityOrderMap _videoQualityOrderMap;
	static FormatDescriptionMap _formatDescriptionMap;
	static std::wstring _watchUri;
	static int _defaultDesiredQuality;

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

	VideoFormatMap getVideoFormatMap(const std::wstring &videoId) const;
	int getDownloadFormatId(const VideoFormatMap &formats, int desiredQualityId, bool withDash) const;

	std::wstring decryptSignature(const std::wstring &encSignature, const std::wstring &signatureScriptUrl) const;
};

