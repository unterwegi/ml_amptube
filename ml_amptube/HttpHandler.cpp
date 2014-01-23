#include "stdafx.h"
#include "HttpHandler.h"

void HttpHandler::doSearch(const std::wstring &query, int page, int maxResults,
	std::function<void(const VideoContainer &results)> finished) const
{
	//Call the callback function with an empty result container if the query is empty
	if (query.empty())
		finished(VideoContainer());
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
			web::http::http_response response)
		{

			if (response.status_code() == web::http::status_codes::OK)
			{
				response.extract_json().then([=](
					web::json::value json)
				{
					web::json::value items = json.get(L"data").get(L"items");
					VideoContainer videos;

					for (auto iter : items)
					{
						videos.push_back(VideoDescription(iter.second));
					}

					finished(videos);
				});
			}
		});
	}
}

pplx::task<void> HttpHandler::retrieveThumbnails(const VideoContainer &videos,
	std::function<void(const std::wstring &videoId, const std::string &data)> thumbnailReady) const
{
	return pplx::create_task([=]() -> void
	{
		for (const auto &video : videos)
		{
			if (!video.getThumbnailUri().empty())
			{
				getRemoteData(video.getThumbnailUri()).then([=](
					pplx::streams::istream responseData)
				{
					concurrency::streams::container_buffer<std::string> buffer;
					responseData.read_to_end(buffer).wait();
					thumbnailReady(video.getId(), buffer.collection());
				}).wait();				
			}
		}
	});
}

pplx::task<pplx::streams::istream> HttpHandler::getRemoteData(const std::wstring &uri) const
{
	web::http::client::http_client client(uri);
	return client.request(web::http::methods::GET).then([=](
		web::http::http_response response) -> pplx::streams::istream
	{
		if (response.status_code() == web::http::status_codes::OK)
		{
			return response.body();
		}
		else
			return pplx::streams::istream();
	});
}