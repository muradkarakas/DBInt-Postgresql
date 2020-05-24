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




#ifdef __cplusplus
extern "C" {
#endif

	void	beginNewTransaction(DBInt_Connection * mkConnection);
	void	endTransaction(DBInt_Connection * mkConnection);
	void	postgresqlAddNewParamEntry(DBInt_Connection *mkDBConnection, DBInt_Statement *stm, char *bindVariableName, int bindVariableType, int bindVariableFormat, void *bindVariableValue, size_t valueLength);
	Oid		writeLobContent(DBInt_Connection *mkDBConnection, const char *filename);
	void*	getLobContent(DBInt_Connection * mkConnection, Oid lobjId, DWORD *sizeOfValue);


#ifdef __cplusplus
}
#endif