/**
 * This file is part of Sodium Language project
 *
 * Copyright © 2020 Murad Karakaþ <muradkarakas@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v3.0
 * as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 *	https://choosealicense.com/licenses/gpl-3.0/
 */

#pragma once

#include <libpq-fe.h>


#ifdef __cplusplus
extern "C" {
#endif

	PGconn			* PQconnectdbUnimplementedFunctionHook();
	void			* libpqUnimplementedFunctionHook();
	ConnStatusType	PQstatus_UnimplementedFunctionHook(const PGconn *conn);
	char			*PQerrorMessage_UnimplementedFunctionHook(const PGconn *conn);
	void			PQfinish_UnimplementedFunctionHook(PGconn *conn);

#ifdef __cplusplus
}
#endif