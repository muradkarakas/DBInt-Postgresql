#include "pch.h"

#include "..\DBInt\db-interface.h"

#include "libpq-fs.h"


void beginNewTransaction(DBInt_Connection * mkConnection) {
	mkConnection->errText = NULL;
	if (mkConnection && mkConnection->connection.postgresqlHandle) {
		PGresult *res = PQexec(mkConnection->connection.postgresqlHandle, "begin");
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		}
		PQclear(res);
	}
}

void endTransaction(DBInt_Connection * mkConnection) {
	mkConnection->errText = NULL; 
	if (mkConnection && mkConnection->connection.postgresqlHandle) {
		PGresult *res = PQexec(mkConnection->connection.postgresqlHandle, "end");
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		}
		PQclear(res);
	}	
}

void* getLobContent(DBInt_Connection * mkConnection, Oid lobjId, DWORD *sizeOfValue) {
	
	PGresult *res = NULL;
	void *retVal = NULL;
	int         lobj_fd;
	*sizeOfValue = 0;

	// create an inversion "object"
	lobj_fd = lo_open(mkConnection->connection.postgresqlHandle, lobjId, INV_READ);
	if (lobj_fd < 0) {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		lo_close(mkConnection->connection.postgresqlHandle, lobj_fd);
		return NULL;
	}

	int size = lo_lseek(mkConnection->connection.postgresqlHandle, lobj_fd, 0, SEEK_END);

	if (size > 0) {
		void *buffer = mkMalloc(mkConnection->heapHandle, size, __FILE__, __LINE__);
		if (buffer != NULL) {
			// set pointer position to the beginning of the stream
			lo_lseek(mkConnection->connection.postgresqlHandle, lobj_fd, 0, SEEK_SET);
			int bytesRead = lo_read(mkConnection->connection.postgresqlHandle, lobj_fd, (char*)buffer, size);
			if (bytesRead == size) {
				*sizeOfValue = size;
				retVal = buffer;
			}
		}
	}
	else {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
	}

	lo_close(mkConnection->connection.postgresqlHandle, lobj_fd);

	return retVal;
}

Oid writeLobContent(DBInt_Connection *mkDBConnection, const char *filename) {

	Oid         lobjId = -1;
	int         lobj_fd;

	// opening file 
	FILE *file;
	fopen_s(&file, filename, "rb");
	if (file == NULL) {
		return lobjId;
	}

	// Getting file size 
	fseek(file, 0L, SEEK_END);
	DWORD fileSize = ftell(file);

	if (fileSize > 0) {

		// Allocating mamory in order to store file content 
		void *buffer = mkMalloc(mkDBConnection->heapHandle, fileSize, __FILE__, __LINE__);
		// setting file pointer to beginning of the file
		fseek(file, 0L, SEEK_SET);

		// reading file into memory allocated
		size_t blocks_read = fread(buffer, fileSize, 1, file);

		if (blocks_read == 1) {
			// memory is filled with file content successfully.

			// creating the large object
			lobjId = lo_creat(mkDBConnection->connection.postgresqlHandle, INV_READ | INV_WRITE);
			if (lobjId == 0) {
				goto FreeResourcesAndExitPoint1;
			}
			// opening and writing file content into large object
			lobj_fd = lo_open(mkDBConnection->connection.postgresqlHandle, lobjId, INV_WRITE);
			DWORD bytesWriten = lo_write(mkDBConnection->connection.postgresqlHandle, lobj_fd, (const char *)buffer, fileSize);
			if (bytesWriten < fileSize) {
				// error occured while writing
			}
			lo_close(mkDBConnection->connection.postgresqlHandle, lobj_fd);
		}

	FreeResourcesAndExitPoint1:
		if (buffer) {
			mkFree(mkDBConnection->heapHandle, buffer);
		}
	}

	// Free resources 
	fclose(file);

	return lobjId;
}

void postgresqlAddNewParamEntry(DBInt_Connection *mkDBConnection, DBInt_Statement *stm, char *bindVariableName, int bindVariableType, int bindVariableFormat, void *bindVariableValue, size_t valueLength) {

	// calculating & resizing memory size
	if (stm->statement.postgresql.bindVariableCount == 0) {
		stm->statement.postgresql.bindVariables = (void**)mkMalloc(mkDBConnection->heapHandle, sizeof(char *), __FILE__, __LINE__);
		stm->statement.postgresql.paramTypes = (Oid *)mkMalloc(mkDBConnection->heapHandle, sizeof(Oid), __FILE__, __LINE__);
		stm->statement.postgresql.paramSizes = (int *)mkMalloc(mkDBConnection->heapHandle, sizeof(int), __FILE__, __LINE__);
		stm->statement.postgresql.paramFormats = (int *)mkMalloc(mkDBConnection->heapHandle, sizeof(int), __FILE__, __LINE__);
	}
	else {
		int memAllocUnitCount = (stm->statement.postgresql.bindVariableCount + 1);
		stm->statement.postgresql.bindVariables = (void**)mkReAlloc(mkDBConnection->heapHandle, stm->statement.postgresql.bindVariables, sizeof(char *) * memAllocUnitCount);
		stm->statement.postgresql.paramTypes = (Oid*)mkReAlloc(mkDBConnection->heapHandle, stm->statement.postgresql.paramTypes, sizeof(Oid) * memAllocUnitCount);
		stm->statement.postgresql.paramSizes = (int*)mkReAlloc(mkDBConnection->heapHandle, stm->statement.postgresql.paramSizes, sizeof(int) * memAllocUnitCount);
		stm->statement.postgresql.paramFormats = (int*)mkReAlloc(mkDBConnection->heapHandle, stm->statement.postgresql.paramFormats, sizeof(int) * memAllocUnitCount);
	}

	// Setting values
	stm->statement.postgresql.bindVariables[stm->statement.postgresql.bindVariableCount] = bindVariableValue;
	stm->statement.postgresql.paramTypes[stm->statement.postgresql.bindVariableCount] = bindVariableType;
	stm->statement.postgresql.paramSizes[stm->statement.postgresql.bindVariableCount] = (int) valueLength;
	stm->statement.postgresql.paramFormats[stm->statement.postgresql.bindVariableCount] = bindVariableFormat;

	// Increasing bind variable count
	stm->statement.postgresql.bindVariableCount++;
}



