// stdafx.cpp : source file that includes just the standard includes
// postgresql-interface.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) BOOL IsPostgreSqLClientDriverLoaded = TRUE;

#ifdef __cplusplus
}
#endif