#include "stdafx.h"
#include "HttpHandler.h"

void HttpHandler::doSearch(const std::wstring &query, int page, int maxResults, SearchCallback callback) const
{
	int startIndex = maxResults * (page - 1) + 1;
	std::wstring requestUri = L"feeds/api/videos?q=" + query
		+ L"&alt=jsonc"
		+ L"&start-index=" + std::to_wstring(startIndex)
		+ L"&max-results=" + std::to_wstring(maxResults)
		+ L"&v=2";

	web::http::client::http_client client(_searchUrl);

	client.request(web::http::methods::GET, web::http::uri::encode_uri(requestUri)).then([=](
		pplx::task<web::http::http_response> responseTask)
	{
		web::http::http_response response = responseTask.get();

		response.extract_json().then([=](
			pplx::task<web::json::value> jsonTask)
		{
			web::json::value json = jsonTask.get();
			web::json::value items = json.get(L"data").get(L"items");			
			VideoContainer videos;

			for (auto iter : items)
			{
				videos.push_back(VideoDescription(iter.second));
			}

			callback(videos);
		});
	});
}
