#pragma once

class VideoDescription
{
public:
	VideoDescription() {}
	VideoDescription(const web::json::value &input);

	std::wstring getId() const { return _id; }
	std::wstring getTitle() const { return _title; }
	std::wstring getUploader() const { return _uploader; }
	std::wstring getUploaded() const { return _uploaded; }
	std::wstring getViewCount() const { return _viewCount; }
	int getDuration() const { return _duration; }
	std::wstring getThumbnailUri() const { return _thumbnailUri; }
	std::wstring getContentUri() const { return _contentUri; }

private:
	std::wstring _id;
	std::wstring _title;
	std::wstring _uploader;
	std::wstring _uploaded;
	std::wstring _viewCount;
	int _duration;
	std::wstring _thumbnailUri;
	std::wstring _contentUri;
};

typedef std::deque<VideoDescription> VideoContainer;

