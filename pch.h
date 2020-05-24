/**
 * This file is part of Sodium Language project
 *
 * Copyright � 2020 Murad Karaka� <muradkarakas@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v3.0
 * as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 *	https://choosealicense.com/licenses/gpl-3.0/
 */

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