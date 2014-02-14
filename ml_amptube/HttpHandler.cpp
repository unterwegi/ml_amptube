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
					web::http::http_response response)
				{
					if (response.status_code() == web::http::status_codes::OK)
					{
						concurrency::streams::container_buffer<std::string> buffer;
						response.body().read_to_end(buffer).wait();
						thumbnailReady(video.getId(), buffer.collection());
					}
				}).wait();				
			}
		}
	});
}

void HttpHandler::startAsyncDownload(const std::wstring &uri, const std::wstring &fileName,
	std::function<void(int progress, bool finished)> progressChanged) const
{
	// Open a stream to the file to write the remote data to
	auto fileBuffer = std::make_shared<pplx::streams::streambuf<uint8_t>>();
	pplx::streams::file_buffer<uint8_t>::open(fileName, std::ios::out).then([=](
		pplx::streams::streambuf<uint8_t> outFile) -> pplx::task<web::http::http_response>
	{
		*fileBuffer = outFile;
		return getRemoteData(uri);
	})
	// Write the response body stream into the file buffer.
	.then([=](web::http::http_response response)
	{
		if (response.status_code() == web::http::status_codes::OK)
		{
			//TODO: Figure out how to get the flv and/or mp4 files. Parse the html of the video page? Use existing tool?
			auto contentLength = response.headers().content_length();
			size_t totalBytesRead = 0;
			int oldPercent = -1;

			while (totalBytesRead < contentLength)
			{
				size_t bytesRead = response.body().read(*fileBuffer, _downloadChunkSize).get();
				if (bytesRead == 0)
					break;

				totalBytesRead += bytesRead;

				int percent = (int) (((double)totalBytesRead / (double)contentLength) * 100);
				if (percent != oldPercent)
					progressChanged(percent, false);
			}
		}
	})
	// Close the file buffer.
	.then([=]
	{
		return fileBuffer->close();
	})
	.then([=]
	{
		progressChanged(100, true);
	});
}

pplx::task<web::http::http_response> HttpHandler::getRemoteData(const std::wstring &uri) const
{
	web::http::client::http_client client(uri);
	return client.request(web::http::methods::GET);
}