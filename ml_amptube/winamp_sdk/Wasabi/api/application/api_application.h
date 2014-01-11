// ----------------------------------------------------------------------------
// Generated by InterfaceFactory [Wed May 07 00:55:56 2003]
//
// File        : api_application.h
// Class       : api_application
// class layer : Dispatchable Interface
// ----------------------------------------------------------------------------

#ifndef __API_APPLICATION_H
#define __API_APPLICATION_H

#include <bfc/dispatch.h>
#include <bfc/platform/guid.h>
#include <bfc/platform/platform.h>
#ifdef _WIN32
#include "ifc_messageprocessor.h"
#endif

// ----------------------------------------------------------------------------

enum
{
  API_APPLICATION_SUCCESS = 0,
  API_APPLICATION_FAILURE = 1,
};

enum
{
  TRANSLATE_MODE_NORMAL = 0,
  TRANSLATE_MODE_GLOBAL = 1,
  TRANSLATE_MODE_CHILD = 2,
};

class NOVTABLE api_application: public Dispatchable
{
protected:
	api_application()
	{}
	~api_application()
	{}
public:
	const wchar_t *main_getAppName(); // returns (e.g.) "Winamp"
	const wchar_t *main_getVersionString(); // returns (e.g.) "Winamp 5.12"
	const wchar_t *main_getVersionNumString();  // returns (e.g.) "5.12"
	unsigned int main_getBuildNumber(); // returns (e.g.) 666
	GUID main_getGUID();
#ifdef _WIN32
	HANDLE main_getMainThreadHandle(); // This actually gives you a DuplicateHandle, so call CloseHandle() when you are done. 
	HINSTANCE main_gethInstance();
#endif
	const wchar_t *main_getCommandLine();
	void main_shutdown(int deferred = TRUE);
	void main_cancelShutdown();
	int main_isShuttingDown();
	const wchar_t *path_getAppPath();
	const wchar_t *path_getUserSettingsPath();
	int app_getInitCount();
	intptr_t app_messageLoopStep();
#ifdef _WIN32
	void app_addMessageProcessor(ifc_messageprocessor *processor);
	void app_removeMessageProcessor(ifc_messageprocessor *processor);
	void app_addModelessDialog(HWND hwnd);
	void app_removeModelessDialog(HWND hwnd);
	/* accelerators are 5.53+ */
	void app_addAccelerators(HWND hwnd, HACCEL *phAccel, INT cAccel, UINT translateMode);
	void app_removeAccelerators(HWND hwnd);
	bool app_translateAccelerators(MSG *msg);
	int app_getAccelerators(HWND hwnd, HACCEL *phAccel, INT cchAccelMax, BOOL bGlobal); // phAccel == NULL && cchAccelMax == 0 -> returns accels count
	/* register window as part of winamp global group (5.54+) */
	void app_registerGlobalWindow(HWND hwnd);
	void app_unregisterGlobalWindow(HWND hwnd);
#endif
	/*
	note: on windows, these two functions DON'T call Set/GetCurrentDirectory, Winamp maintains it's own path
	 because calling SetCurrentDirectory locks the folder
	Added for 5.34
	*/
	const wchar_t *path_getWorkingPath(); // useful to call for populating lpstrInitialDir in GetOpenFileName
	void path_setWorkingPath(const wchar_t *newPath); // useful to call for populating lpstrInitialDir in GetOpenFileName

	/*
	The following three function return you unique IDs you can use if you need
	They are created anonymously, so information cannot be tracked back to a specific person
	The main reason for their existence is that a few third party libraries require them
	and some online media providers require this info for billing.
	You can call this functions with a pointer to any 16 byte data structure cast to a GUID *
	Added for 5.35
	*/
	// returns an ID unique to this computer, but not unique to the logged in user (two windows accts would share this ID)
	int GetMachineID(GUID *id);
	// returns an ID unique to this user.  Another user logged in to the computer will have a different ID
	// note that if Winamp was installed with "shared settings", the IDs of multiple users will probably be identical
	// as we're just storing it in winamp.ini for now
	int GetUserID(GUID *id);
	// returns a unique ID for this session.  Generated on the fly the first time someone calls this function
	int GetSessionID(GUID *id);

	/* 5.54 + */
	size_t AllocateThreadStorage(); // returns an index, -1 for error
	void *GetThreadStorage(size_t index);
	void SetThreadStorage(size_t index, void *value);

protected:
	enum
	{
	  API_APPLICATION_MAIN_GETAPPNAME = 10,
	  API_APPLICATION_MAIN_GETVERSIONSTRING = 20,
	  API_APPLICATION_MAIN_GETVERSIONSTRING2 = 21,
	  API_APPLICATION_MAIN_GETBUILDNUMBER = 30,
	  API_APPLICATION_MAIN_GETGUID = 40,
	  API_APPLICATION_MAIN_GETMAINTHREADHANDLE = 50,
	  API_APPLICATION_MAIN_GETHINSTANCE = 60,
	  API_APPLICATION_MAIN_GETCOMMANDLINE = 70,
	  API_APPLICATION_MAIN_SHUTDOWN = 80,
	  API_APPLICATION_MAIN_CANCELSHUTDOWN = 90,
	  API_APPLICATION_MAIN_ISSHUTTINGDOWN = 100,
	  API_APPLICATION_PATH_GETAPPPATH = 110,
	  API_APPLICATION_PATH_GETUSERSETTINGSPATH = 120,
	  API_APPLICATION_APP_GETINITCOUNT = 130,
	  API_APPLICATION_APP_MESSAGELOOPSTEP = 140,
	  API_APPLICATION_APP_ADDMESSAGEPROCESSOR = 150,
	  API_APPLICATION_APP_REMOVEMESSAGEPROCESSOR = 160,
	  API_APPLICATION_APP_ADDMODELESSDIALOG = 170,
	  API_APPLICATION_APP_REMOVEMODELESSDIALOG = 180,
	  API_APPLICATION_PATH_GETWORKINGPATH = 190,
	  API_APPLICATION_PATH_SETWORKINGPATH = 200,
	  API_APPLICATION_GETMACHINEID = 210,
	  API_APPLICATION_GETUSERID = 220,
	  API_APPLICATION_GETSESSIONID = 230,
	  API_APPLICATION_APP_ADDACCELERATORS = 240,
	  API_APPLICATION_APP_REMOVEACCELERATORS = 250,
	  API_APPLICATION_APP_TRANSLATEACCELERATORS = 260,
	  API_APPLICATION_APP_GETACCELERATORS = 270,
	  API_APPLICATION_APP_REGISTERGLOBALWINDOW = 280,
	  API_APPLICATION_APP_UNREGISTERGLOBALWINDOW = 290,
		API_APPLICATION_ALLOCATETHREADSTORAGE = 300,
		API_APPLICATION_GETTHREADSTORAGE = 310,
		API_APPLICATION_SETTHREADSTORAGE = 320,
	};
};

// ----------------------------------------------------------------------------

inline const wchar_t *api_application::main_getAppName()
{
	const wchar_t *__retval = _call(API_APPLICATION_MAIN_GETAPPNAME, (const wchar_t *)0);
	return __retval;
}

inline const wchar_t *api_application::main_getVersionString()
{
	const wchar_t *__retval = _call(API_APPLICATION_MAIN_GETVERSIONSTRING, (const wchar_t *)0);
	return __retval;
}

inline const wchar_t *api_application::main_getVersionNumString()
{
	return _call(API_APPLICATION_MAIN_GETVERSIONSTRING2, (const wchar_t *)0);
}

inline unsigned int api_application::main_getBuildNumber()
{
	return _call(API_APPLICATION_MAIN_GETBUILDNUMBER, 0);
}

inline GUID api_application::main_getGUID()
{
	GUID __retval = _call(API_APPLICATION_MAIN_GETGUID, INVALID_GUID);
	return __retval;
}
#ifdef _WIN32
inline HANDLE api_application::main_getMainThreadHandle()
{
	HANDLE __retval = _call(API_APPLICATION_MAIN_GETMAINTHREADHANDLE, (HANDLE)NULL);
	return __retval;
}

inline HINSTANCE api_application::main_gethInstance()
{
	HINSTANCE __retval = _call(API_APPLICATION_MAIN_GETHINSTANCE, (HINSTANCE)NULL);
	return __retval;
}
#endif

inline const wchar_t *api_application::main_getCommandLine()
{
	const wchar_t *__retval = _call(API_APPLICATION_MAIN_GETCOMMANDLINE, (const wchar_t *)0);
	return __retval;
}

inline void api_application::main_shutdown(int deferred)
{
	_voidcall(API_APPLICATION_MAIN_SHUTDOWN, deferred);
}

inline void api_application::main_cancelShutdown()
{
	_voidcall(API_APPLICATION_MAIN_CANCELSHUTDOWN);
}

inline int api_application::main_isShuttingDown()
{
	int __retval = _call(API_APPLICATION_MAIN_ISSHUTTINGDOWN, (int)0);
	return __retval;
}

inline const wchar_t *api_application::path_getAppPath()
{
	const wchar_t *__retval = _call(API_APPLICATION_PATH_GETAPPPATH, (const wchar_t *)0);
	return __retval;
}

inline const wchar_t *api_application::path_getUserSettingsPath()
{
	return _call(API_APPLICATION_PATH_GETUSERSETTINGSPATH, (const wchar_t *)0);
}

inline int api_application::app_getInitCount()
{
	int __retval = _call(API_APPLICATION_APP_GETINITCOUNT, 0);
	return __retval;
}

inline intptr_t api_application::app_messageLoopStep()
{
	return _call(API_APPLICATION_APP_MESSAGELOOPSTEP, (intptr_t)1);
}
#ifdef _WIN32

inline void api_application::app_addMessageProcessor(ifc_messageprocessor *processor)
{
	_voidcall(API_APPLICATION_APP_ADDMESSAGEPROCESSOR, processor);
}

inline void api_application::app_removeMessageProcessor(ifc_messageprocessor *processor)
{
	_voidcall(API_APPLICATION_APP_REMOVEMESSAGEPROCESSOR, processor);
}

inline void api_application::app_addModelessDialog(HWND hwnd)
{
	_voidcall(API_APPLICATION_APP_ADDMODELESSDIALOG, hwnd);
}

inline void api_application::app_removeModelessDialog(HWND hwnd)
{
	_voidcall(API_APPLICATION_APP_REMOVEMODELESSDIALOG, hwnd);
}

inline void api_application::app_addAccelerators(HWND hwnd, HACCEL *phAccel, INT cAccel, UINT translateMode)
{
	_voidcall(API_APPLICATION_APP_ADDACCELERATORS, hwnd, phAccel, cAccel, translateMode);
}
inline void api_application::app_removeAccelerators(HWND hwnd)
{
	_voidcall(API_APPLICATION_APP_REMOVEACCELERATORS, hwnd);
}

inline bool api_application::app_translateAccelerators(MSG *msg)
{
	return _call(API_APPLICATION_APP_TRANSLATEACCELERATORS, (bool)false, msg);
}

inline int api_application::app_getAccelerators(HWND hwnd, HACCEL *phAccel, INT cchAccelMax, BOOL bGlobal)
{
	return _call(API_APPLICATION_APP_GETACCELERATORS, (int)0, hwnd, phAccel, cchAccelMax, bGlobal);
}

inline void api_application::app_registerGlobalWindow(HWND hwnd)
{
	_voidcall(API_APPLICATION_APP_REGISTERGLOBALWINDOW, hwnd);
}

inline void api_application::app_unregisterGlobalWindow(HWND hwnd)
{
	_voidcall(API_APPLICATION_APP_UNREGISTERGLOBALWINDOW, hwnd);
}


#endif

inline const wchar_t *api_application::path_getWorkingPath()
{
	return _call(API_APPLICATION_PATH_GETWORKINGPATH, (wchar_t *)0);
}

inline void api_application::path_setWorkingPath(const wchar_t *newPath)
{
	_voidcall(API_APPLICATION_PATH_SETWORKINGPATH, newPath);
}

inline int api_application::GetMachineID(GUID *id)
{
	return _call(API_APPLICATION_GETMACHINEID, (int)API_APPLICATION_FAILURE, id);
}

inline int api_application::GetUserID(GUID *id)
{
	return _call(API_APPLICATION_GETUSERID, (int)API_APPLICATION_FAILURE, id);
}

inline int api_application::GetSessionID(GUID *id)
{
	return _call(API_APPLICATION_GETSESSIONID, (int)API_APPLICATION_FAILURE, id);
}


inline size_t api_application::AllocateThreadStorage()
{
	return _call(API_APPLICATION_ALLOCATETHREADSTORAGE, (size_t)-1);
}

inline void *api_application::GetThreadStorage(size_t index)
{
	return _call(API_APPLICATION_GETTHREADSTORAGE, (void *)0, index);
}

inline void api_application::SetThreadStorage(size_t index, void *value)
{
	_voidcall(API_APPLICATION_SETTHREADSTORAGE, index, value);
}


// ----------------------------------------------------------------------------


// {23B96771-09D7-46d3-9AE2-20DCEA6C86EA}
static const GUID applicationApiServiceGuid =
  {
    0x23b96771, 0x9d7, 0x46d3, { 0x9a, 0xe2, 0x20, 0xdc, 0xea, 0x6c, 0x86, 0xea }
  };

extern api_application *applicationApi;

#endif // __API_APPLICATION_H
