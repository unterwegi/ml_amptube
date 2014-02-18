#pragma once
#include <string>
#include <exception>

class VideoFormatParseException : public std::exception
{
public:
	VideoFormatParseException(std::wstring reason) : _reason(reason) {}

	std::wstring getReason(){ return _reason; }

private:
	std::wstring _reason; 
};