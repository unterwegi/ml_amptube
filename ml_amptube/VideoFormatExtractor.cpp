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
	try
	{
		HttpHandler::instance().getRemoteData(_watchUri + videoId).then([=]
			(web::http::http_response response)
		{
			std::wstring content = response.extract_string().get();
			boost::wregex fmtRegex(L"\"url_encoded_fmt_stream_map\":\\s*\"([^\"]+)\"", boost::regex_constants::extended);
			boost::wsmatch fmtResults;

			//TODO: Investigate why the regex is not working
			boost::regex_match(content, fmtResults, fmtRegex);
			if (!fmtResults.empty())
			{
				std::wstring videoFormats = fmtResults.prefix().str();
			}

		}).wait();
	}
	catch (std::exception &e)
	{
		MessageBoxA(NULL, e.what(), "Exception", MB_OK);
	}

	return formatMap;
}
