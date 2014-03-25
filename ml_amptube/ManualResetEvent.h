#pragma once
#include "stdafx.h"

class ManualResetEvent
{
public:
	ManualResetEvent() : _eventHandle(NULL)
	{
		_eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~ManualResetEvent()
	{
		if (_eventHandle)
		{
			CloseHandle(_eventHandle);
			_eventHandle = NULL;
		}
	}

	void set()
	{
		std::lock_guard<std::mutex> lock(_lockMutex);
		SetEvent(_eventHandle);
	}

	void unset()
	{
		std::lock_guard<std::mutex> lock(_lockMutex);
		ResetEvent(_eventHandle);
	}

	bool wait(int timeout) const
	{
		int result = WaitForSingleObject(_eventHandle, timeout);
		return (result == WAIT_OBJECT_0);
	}

private:

	std::mutex _lockMutex;
	HANDLE _eventHandle;
};

