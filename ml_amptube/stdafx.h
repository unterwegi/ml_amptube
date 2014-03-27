#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN 
#define OEMRESOURCE
#define NOMINMAX
#define _SCL_SECURE_NO_WARNINGS
// Windows Headers
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

// CRT + STL Headers
#include <iostream>
#include <string>
#include <codecvt>
#include <algorithm>
#include <future>
#include <set>

// C++ REST SDK Headers
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

// Boost Headers
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

// Winamp Headers
#include <Winamp/wa_ipc.h>
#include <Winamp/wa_dlg.h>
#include <gen_ml/ml.h>
#include <gen_ml/childwnd.h>
#include <gen_ml/ml_ipc_0313.h>
#include <api/service/api_service.h>
#include <api/service/waServiceFactory.h>
#include <api/application/api_application.h>
#include <ml_local/api_mldb.h>
#include <Agave/Language/api_language.h>
#include <bfc/dispatch.h>
