#include "stdafx.h"
#include "HttpHandler.h"

void HttpHandler::doSearch(const std::wstring &query, int page, int maxResults, SearchCallback callback) const
{
	//Call the callback function with an empty result container if the query is empty
	if (query.empty())
		callback(VideoContainer());
	else
	{
		int startIndex = maxResults * (page - 1) + 1;
		std::wstring requestUri = L"feeds/api/videos?q=" + query
			+ L"&alt=jsonc"
			+ L"&start-index=" + std::to_wstring(startIndex)
			+ L"&max-results=" + std::to_wstring(maxResults)
			+ L"&v=2";

		web::http::client::http_client client(_searchUrl);

		client.request(web::http::methods::GET, web::http::uri::encode_uri(requestUri)).then([=](
			//pplx::task<web::http::http_response> responseTask)
			web::http::http_response response)
		{
			//web::http::http_response response = responseTask.get();

			if (response.status_code() == web::http::status_codes::OK)
			{
				response.extract_json().then([=](
					//pplx::task<web::json::value> jsonTask)
					web::json::value json)
				{
					//web::json::value json = jsonTask.get();
					web::json::value items = json.get(L"data").get(L"items");
					VideoContainer videos;

					for (auto iter : items)
					{
						videos.push_back(VideoDescription(iter.second));
					}

					callback(videos);
				});
			}
		});
	}
}

void HttpHandler::retrieveThumbnails(const VideoContainer &videos, ThumbnailRetrievedCallback callback) const
{
	//pplx::critical_section cs;

	for (const auto &video : videos)
	{
		if (!video.getThumbnailUri().empty())
		{
			web::http::client::http_client client(video.getThumbnailUri());
			client.request(web::http::methods::GET).then([=](
				//pplx::task<web::http::http_response> responseTask)
				web::http::http_response response)
			{
				//web::http::http_response response = responseTask.get();
				std::wstring imgPath;

				if (response.status_code() == web::http::status_codes::OK)
				{
					imgPath = PluginProperties::instance().getProperty(L"cachePath")
						+ L"\\" + video.getId() + L".jpg";
					concurrency::streams::container_buffer<std::string> buffer;
					response.body().read_to_end(buffer).wait();
					std::ofstream fileStream;
					fileStream.open(imgPath, std::ios::out | std::ios::trunc | std::ios::binary);
					fileStream << buffer.collection();
					fileStream.close();
				}

				callback(video.getListItemIdx(), imgPath);
			}).wait();
		}
	}
}