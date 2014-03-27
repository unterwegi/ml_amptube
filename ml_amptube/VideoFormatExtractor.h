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
		FormatDescription(const std::wstring &containerName, int qualityId, bool usesDash)
			: _containerName(containerName), _qualityId(qualityId), _usesDash(usesDash) {}

		std::wstring getContainerName() const { return _containerName; }
		int getQualityId() const { return _qualityId; }
		bool isDash() const { return _usesDash; }

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

	class VideoFormatInfo
	{
	public:
		VideoFormatInfo() : _encoded(false) {}
		VideoFormatInfo(const std::wstring &url, const std::wstring &signature, const bool encoded, const std::wstring &scriptUrl)
			: _url(url), _signature(signature), _scriptUrl(scriptUrl) {}

		std::wstring getUrl() const { return _url; }
		std::wstring getSignature() const { return _signature; }
		bool IsEncoded() const { return _encoded; }
		std::wstring getScriptUrl() const { return _scriptUrl; }
	private:
		std::wstring _url;
		std::wstring _signature;
		bool _encoded;
		std::wstring _scriptUrl;
	};

	typedef std::map<int, VideoFormatInfo>		VideoFormatMap;

	typedef std::set<std::wstring>				ExtensionsList;

	///<summary>
	/// Returns a reference to the static class instance.</summary>
	///<returns>
	/// A reference to the static class instance.</returns>
	static VideoFormatExtractor &instance()
	{
		static VideoFormatExtractor _instance;
		return _instance;
	}

	VideoQualityOrderMap getVideoQualitiesOrderMap() const { return _videoQualityOrderMap; }
	int getDefaultDesiredQuality() const { return _defaultDesiredQuality; }
	FormatDescriptionMap getFormatDescriptionMap() const { return _formatDescriptionMap; }

	ExtensionsList getAvailableExtensions() const;

	~VideoFormatExtractor(){}

	std::wstring startDownload(const VideoDescription &video, 
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

	std::wstring getVideoDownloadUrl(const VideoFormatInfo &videoFormatInfo) const;
	std::wstring decryptSignature(const std::wstring &encSignature, const std::wstring &signatureScriptUrl) const;
};

