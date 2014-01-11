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
	_thumbnailUri = input.get(L"thumbnail").get(L"default").to_string();
	//_contentUri = input.get(L"content").get(0).to_string();
}

