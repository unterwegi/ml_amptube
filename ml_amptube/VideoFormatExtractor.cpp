#include "stdafx.h"
#include "VideoFormatExtractor.h"
#include "HttpHandler.h"

VideoFormatExtractor::FormatDescriptionMap VideoFormatExtractor::_formatDescriptionMap = {
	{ 5, VideoFormatExtractor::FormatDescription(L"FLV", L"240p", false) },
	{ 18, VideoFormatExtractor::FormatDescription(L"MP4", L"270p/360p", false) },
	{ 22, VideoFormatExtractor::FormatDescription(L"MP4", L"720p", false) },
	{ 133, VideoFormatExtractor::FormatDescription(L"MP4", L"240p", true) },
	{ 134, VideoFormatExtractor::FormatDescription(L"MP4", L"360p", true) },
	{ 135, VideoFormatExtractor::FormatDescription(L"MP4", L"480p", true) },
	{ 136, VideoFormatExtractor::FormatDescription(L"MP4", L"720p", true) },
};

std::wstring VideoFormatExtractor::_watchUri = L"https://www.youtube.com/watch?v=";

VideoFormatExtractor::VideoFormatMap VideoFormatExtractor::getVideoFormatMap(const std::wstring &videoId) const
{
	VideoFormatExtractor::VideoFormatMap formatMap;

	HttpHandler::instance().getRemoteData(_watchUri + videoId).then([&]
		(web::http::http_response response)
	{
		std::wstring responseBody = response.extract_string().get();
		boost::wregex videoFormatRegex(L"\"url_encoded_fmt_stream_map\":\\s*\"([^\"]+)\"", boost::regex_constants::perl);
		boost::wregex videoAdaptFormatRegex(L"\"adaptive_fmts\":\\s*\"([^\"]+)\"", boost::regex_constants::perl);
		boost::wsmatch matches;

		std::wstring videoFormats, videoAdaptFormats;

		if (boost::regex_search(responseBody, matches, videoFormatRegex))
		{
			videoFormats.assign(matches[1].first, matches[1].second);

			if (boost::regex_search(responseBody, matches, videoAdaptFormatRegex))
				videoAdaptFormats.assign(matches[1].first, matches[1].second);
		}
		else
			throw VideoFormatParseException(L"No url_encoded_fmt_stream property was found. Something must have changed behind the scenes ...");

		//Start the formats parsing
		//Determine the used separators
		std::wstring sep1 = L"%2C", sep2 = L"%26", sep3 = L"%3D";
		
		if (boost::contains(videoFormats, L","))
		{
			sep1 = L",";
			sep2 = (boost::contains(videoFormats, L"&")) ? L"&" : L"\\u0026";
			sep3 = L"=";
		}

		if (!videoAdaptFormats.empty())
			videoFormats.append(sep1).append(videoAdaptFormats);

		std::vector<std::wstring> formatElems;
		boost::iter_split(formatElems, videoFormats, boost::first_finder(sep1));

		for (const auto &formatElem : formatElems)
		{
			std::map<std::wstring, std::wstring> formatValues;

			std::vector<std::wstring> formatValueEntries;
			boost::iter_split(formatValueEntries, formatElem, boost::first_finder(sep2));

			for (const auto &formatValueEntry : formatValueEntries)
			{
				std::vector<std::wstring> entryPair;
				boost::iter_split(entryPair, formatValueEntry, boost::first_finder(sep3));

				if (entryPair.size() == 2)
					formatValues[entryPair.at(0)] = entryPair[1];
			}

			if (formatValues.find(L"url") != formatValues.end() && formatValues.find(L"itag") != formatValues.end())
			{
				std::wstring url = web::http::uri::decode(formatValues[L"url"]);
				boost::replace_all(url, L"\\/", L"/");
				boost::replace_all(url, L"\\u0026", L"&");
				url = web::http::uri::decode(url);

				int itag = std::stoi(formatValues[L"itag"]);

				std::wstring signature;

				if (formatValues.find(L"sig") != formatValues.end())
					signature = formatValues[L"sig"];
				else if (formatValues.find(L"signature") != formatValues.end())
					signature = formatValues[L"signature"];

				if (!signature.empty())
					url.append(L"&signature=").append(signature);
				else
				{
					//TODO: Handle encrypted signatures in the formatValues[L"s"] field
				}

				if (!boost::icontains(url, L"ratebypass"))
					url.append(L"&ratebypass=yes");

				formatMap[itag] = url;
			}
		}
	}).wait();

	return formatMap;
}
