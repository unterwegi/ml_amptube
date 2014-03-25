#pragma once
#include "VideoDescription.h"
#include "ManualResetEvent.h"

///<summary>
/// Handles all HTTP related functionality. Uses the singleton pattern.</summary>
class HttpHandler
{
public:
	///<summary>
	/// Returns a reference to the static class instance.</summary>
	///<returns>
	/// A reference to the static class instance.</returns>
	static HttpHandler &instance()
	{ 
		static HttpHandler _instance;
		return _instance;
	}

	~HttpHandler(){}

	void cancelAsyncTasks() const;
	std::shared_ptr<ManualResetEvent> getCancelEvent() const;
	
	///<summary>
	/// Performs an asynchronous search and calls the supplied callback when the search is done.</summary>
	///<param name="query"> The search query.</param>
	///<param name="page"> Tells the search, which result page should be returned. Starts with 1.</param>
	///<param name="maxResult"> Maximum number of returned search results.</param>
	///<param name="finished"> callable target (function pointer, lambda, ...), which is called when the search is done.</param>
	void doSearch(const std::wstring &query, int page, int maxResults,
		std::function<void(const VideoContainer &results)> finished) const;

	pplx::task<void> retrieveThumbnails(const VideoContainer &videos,
		std::function<void(const std::wstring &videoId, const std::string &data)> thumbnailReady) const;

	void HttpHandler::startAsyncDownload(const std::wstring &uri, const std::wstring &fileName,
		std::function<void(int progress, bool finished)> progressChanged);

	pplx::task<web::http::http_response> getRemoteData(const std::wstring &uri) const;
private:
	///<summary>
	/// The base URL for search operations</summary>
	static std::wstring _searchUrl;
	static size_t _downloadChunkSize;

	std::deque<pplx::task<void>> _activeDownloads;

	std::shared_ptr<ManualResetEvent> _cancelEvent;

	///<summary>
	/// Private constructor. Needed for proper singleton.</summary>
	HttpHandler() : _cancelEvent(std::shared_ptr<ManualResetEvent>(new ManualResetEvent)) {}

	///<summary>
	/// Private copy constructor. Needed for proper singleton.</summary>
	HttpHandler(const HttpHandler&) {}
	///<summary>
	/// Private assignment operator. Needed for proper singleton.</summary>
	HttpHandler& operator=(const HttpHandler&) {}
};

