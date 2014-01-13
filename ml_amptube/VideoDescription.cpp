#include "stdafx.h"
#include "VideoDescription.h"

VideoDescription::VideoDescription(const web::json::value &input)
{
	_id = input.get(L"id").as_string();
	_title = input.get(L"title").as_string();
	_uploader = input.get(L"uploader").as_string();
	_uploaded = input.get(L"uploaded").as_string();
	_viewCount = input.get(L"viewCount").to_string();
	_duration = input.get(L"duration").as_integer();

	if (input.get(L"thumbnail").has_field(L"hqDefault"))
		_thumbnailUri = input.get(L"thumbnail").get(L"hqDefault").as_string();
	else if (input.get(L"thumbnail").has_field(L"sqDefault"))
		_thumbnailUri = input.get(L"thumbnail").get(L"sqDefault").as_string();

	if (input.get(L"content").has_field(L"5"))
		_contentUri = input.get(L"content").get(L"5").as_string();
}

