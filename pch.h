// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "targetver.h"

// Windows Header Files:
#include <windows.h>

#define DLL_EXPORT	__declspec(dllexport)

// TODO: reference additional headers your program requires here


#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) BOOL IsPostgreSqLClientDriverLoaded;

#ifdef __cplusplus
}
#endif