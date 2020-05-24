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

#include "pch.h"

#include "DBInt_Postgresql_delayLoaded_DLL_FuncImps.h"


PGconn  *PQconnectdbUnimplementedFunctionHook() {
	return NULL;
}

void *libpqUnimplementedFunctionHook() {
	return NULL;
}

void PQfinish_UnimplementedFunctionHook(PGconn *conn) {

}

ConnStatusType	PQstatus_UnimplementedFunctionHook(const PGconn *conn) {
	return CONNECTION_BAD;
}

char *PQerrorMessage_UnimplementedFunctionHook(const PGconn *conn) {
	return "Either postgresql client library \"libpq.dll\" not found or \"PQerrorMessage\" is not exposed/found in it.";
}