#include "stdafx.h"
#include "HttpHandler.h"

std::wstring HttpHandler::_searchUrl = L"https://gdata.youtube.com/"; //URL to Youtube Search API
size_t HttpHandler::_downloadChunkSize = 512 * 1024; // 512 KB chunk size

void HttpHandler::cancelAsyncTasks() const
{
	_cancellationToken.cancel();
	for (const auto &task : _activeDownloads)
	{
		task.wait();
	}
}

pplx::cancellation_token HttpHandler::getCancellationToken() const
{
	return _cancellationToken.get_token();
}

void HttpHandler::doSearch(const std::wstring &query, int page, int maxResults,
	std::function<void(const VideoContainer &results)> finished) const
{
	auto cancelToken = _cancellationToken.get_token();

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
			if (cancelToken.is_canceled())
				pplx::cancel_current_task();

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
				}, _cancellationToken.get_token());
			}
		}, cancelToken);
	}
}

pplx::task<void> HttpHandler::retrieveThumbnails(const VideoContainer &videos,
	std::function<void(const std::wstring &videoId, const std::string &data)> thumbnailReady) const
{
	auto cancelToken = _cancellationToken.get_token();

	return pplx::create_task([=]() -> void
	{
		for (const auto &video : videos)
		{
			if (cancelToken.is_canceled())
				pplx::cancel_current_task();

			if (!video.getThumbnailUri().empty())
			{
				getRemoteData(video.getThumbnailUri()).then([=](
					web::http::http_response response)
				{
					if (cancelToken.is_canceled())
						pplx::cancel_current_task();

					if (response.status_code() == web::http::status_codes::OK)
					{
						concurrency::streams::container_buffer<std::string> buffer;
						response.body().read_to_end(buffer).wait();
						thumbnailReady(video.getId(), buffer.collection());
					}
				}, cancelToken).wait();
			}
		}
	});
}

void HttpHandler::startAsyncDownload(const std::wstring &uri, const std::wstring &fileName,
	std::function<void(int progress, bool finished)> progressChanged)
{
	auto cancelToken = _cancellationToken.get_token();
	
	//file name for the target file during the download (.incomplete appended to fileName)
	std::wstring fileNameIncomplete = fileName + L".incomplete";

	// Open a stream to the file to write the remote data to
	auto fileBuffer = std::make_shared<pplx::streams::streambuf<uint8_t>>();
	auto task = pplx::streams::file_buffer<uint8_t>::open(fileNameIncomplete, std::ios::out).then([=](
		pplx::streams::streambuf<uint8_t> outFile) -> pplx::task<web::http::http_response>
	{
		if (cancelToken.is_canceled())
			pplx::cancel_current_task();

		*fileBuffer = outFile;
		return getRemoteData(uri);
	}, cancelToken)
	// Write the response body stream into the file buffer.
	.then([=](web::http::http_response response)
	{
		if (response.status_code() == web::http::status_codes::OK)
		{
			auto contentLength = response.headers().content_length();
			size_t totalBytesRead = 0;
			int oldPercent = -1;

			while (totalBytesRead < contentLength)
			{
				if (cancelToken.is_canceled())
					pplx::cancel_current_task();

				size_t bytesRead = response.body().read(*fileBuffer, _downloadChunkSize).get();
				if (bytesRead == 0)
					break;

				totalBytesRead += bytesRead	;

				int percent = (int) (((double)totalBytesRead / (double)contentLength) * 100);
				if (percent != oldPercent)
					progressChanged(percent, false);
			}
		}
	}, cancelToken)
	// Close the file buffer and remove the .incomplete suffix from the file
	.then([=]
	{
		fileBuffer->close().wait();
		if (cancelToken.is_canceled())
			pplx::cancel_current_task();

		boost::filesystem::rename(fileNameIncomplete, fileName);
		progressChanged(100, true);
	}, cancelToken);

	_activeDownloads.push_back(task);

	task.then([this, task]
	{
		auto iter = std::find(_activeDownloads.begin(), _activeDownloads.end(), task);
		if (iter != _activeDownloads.end())
			_activeDownloads.erase(iter);
	});
}

pplx::task<web::http::http_response> HttpHandler::getRemoteData(const std::wstring &uri) const
{
	web::http::client::http_client client(uri);
	return client.request(web::http::methods::GET);
}