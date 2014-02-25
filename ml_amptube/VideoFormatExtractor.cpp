#include "stdafx.h"
#include "VideoFormatExtractor.h"
#include "HttpHandler.h"
#include "ml_amptube.h"

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
		boost::wregex scriptUrlRegex(L"\"js\":\\s*\"([^\"]+)\"", boost::regex_constants::perl);
		boost::wsmatch matches;

		std::wstring videoFormats, videoAdaptFormats, scriptUrl;

		if (boost::regex_search(responseBody, matches, videoFormatRegex))
		{
			videoFormats.assign(matches[1].first, matches[1].second);

			if (boost::regex_search(responseBody, matches, videoAdaptFormatRegex))
				videoAdaptFormats.assign(matches[1].first, matches[1].second);
		}
		else
			throw VideoFormatParseException(L"No url_encoded_fmt_stream property was found. Something must have changed behind the scenes ...");

		if (boost::regex_search(responseBody, matches, scriptUrlRegex))
		{
			scriptUrl.assign(matches[1].first, matches[1].second);
			boost::replace_all(scriptUrl, L"\\", L"");

			if (scriptUrl.find(L"//") == 0)
				scriptUrl = L"https:" + scriptUrl;
		}

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
				else if (formatValues.find(L"s") != formatValues.end())
					url.append(L"&signature=").append(decryptSignature(formatValues[L"s"], scriptUrl));

				if (!boost::icontains(url, L"ratebypass"))
					url.append(L"&ratebypass=yes");

				formatMap[itag] = url;
			}
		}
	}).wait();

	return formatMap;
}


std::wstring VideoFormatExtractor::decryptSignature(const std::wstring &encSignature, const std::wstring &signatureScriptUrl) const
{
	if (encSignature.empty() || signatureScriptUrl.empty()) return L"";
	
	std::vector<int> decodeArray;
	std::size_t signatureLength = 81;

	HttpHandler::instance().getRemoteData(signatureScriptUrl).then([&]
		(web::http::http_response response)
	{
		std::wstring scriptCode = response.extract_string().get();
		
		boost::wregex functionNameRegex(L"\\.signature\\s*=\\s*(\\w+)\\(\\w+\\)", boost::regex_constants::perl);
		boost::wsmatch matches;

		if (boost::regex_search(scriptCode, matches, functionNameRegex))
		{
			std::wstring functionName(matches[1].first, matches[1].second);
			boost::wregex functionCodeRegex(L"function " + functionName +
				L"\\s*\\(\\w+\\)\\s*{\\w+=\\w+\\.split\\(\"\"\\);(.+);return \\w+\\.join",
				boost::regex_constants::perl);

			if (boost::regex_search(scriptCode, matches, functionCodeRegex))
			{
				std::wstring functionCode(matches[1].first, matches[1].second);

				boost::wregex sliceRegex(L"slice\\s*\\(\\s*(.+)\\s*\\)",
					boost::regex_constants::perl);
				boost::wregex swapRegex(L"\\w+\\s*\\(\\s*\\w+\\s*,\\s*([0-9]+)\\s*\\)",
					boost::regex_constants::perl);
				boost::wregex inlineRegex(L"\\w+\\[0\\]\\s*=\\s*\\w+\\[([0-9]+)\\s*%\\s*\\w+\\.length\\]",
					boost::regex_constants::perl);

				std::vector<std::wstring> codePieces;
				boost::iter_split(codePieces, functionCode, boost::first_finder(L";"));
				for (std::size_t i = 0; i < codePieces.size(); ++i)
				{
					boost::trim(codePieces[i]);
					if (!codePieces[i].empty())
					{
						if (boost::contains(codePieces[i], L"slice"))
						{
							if (boost::regex_search(codePieces[i], matches, sliceRegex))
							{
								std::wstring slice(matches[1].first, matches[1].second);
								int sliceInt;

								if (TryParse(slice, sliceInt))
								{
									decodeArray.push_back(-sliceInt);
									signatureLength += sliceInt;
								}
								else return;
							}
							else return;
						}
						else
						{
							if (boost::contains(codePieces[i], L"reverse"))
								decodeArray.push_back(0);
							else
							{
								if (boost::contains(codePieces[i], L"[0]"))
								{
									if ((i + 2) < codePieces.size()
										&& boost::contains(codePieces[i + 1], L".length")
										&& boost::contains(codePieces[i + 1], L"[0]"))
									{
										if (boost::regex_search(codePieces[i], matches, inlineRegex))
										{
											std::wstring inlineStr(matches[1].first, matches[1].second);
											int inlineInt;

											if (TryParse(inlineStr, inlineInt))
											{
												decodeArray.push_back(inlineInt);
												i+=2;
											}
											else return;
										}
										else return;
									}
									else return;
								}
								else
								{
									if (boost::contains(codePieces[i], L","))
									{
										if (boost::regex_search(codePieces[i], matches, swapRegex))
										{
											std::wstring swapStr(matches[1].first, matches[1].second);
											int swapInt;

											if (TryParse(swapStr, swapInt))
											{
												decodeArray.push_back(swapInt);
											}
											else return;
										}
										else return;
									}
									else return;
								}
							}
						}
					}
				}
			}
		}
	}).wait();

	if (encSignature.length() == signatureLength && !decodeArray.empty())
	{
		std::vector<wchar_t> sigArray;
		std::copy(encSignature.begin(), encSignature.end(), sigArray.begin());

		for (const auto &act : decodeArray)
		{
			if (act > 0)
			{
				wchar_t temp = sigArray[0];
				sigArray[0] = sigArray[act%sigArray.size()];
				sigArray[act] = temp;
			}
			else
			{
				if (act == 0)
				{
					std::reverse(sigArray.begin(), sigArray.end());
				}
				else
				{
					std::vector<wchar_t> bufArray(&sigArray[-act], &(*sigArray.end()));
					sigArray = std::move(bufArray);
				}
			}
		}

		std::wstring decSignature(sigArray.begin(), sigArray.end());

		return (decSignature.length() == 81) ? decSignature : encSignature;
	}
	else return encSignature;
}