#include "stdafx.h"

#include "..\DBInt\db-interface.h"


#include "postgresql-interface.h"
#include "postgresql-utilities.h"

#include <libpq-fe.h>


POSTGRESQLINTERFACE_API
char* 
postgresqlGetPrimaryKeyColumn(
	DBInt_Connection* mkDBConnection, 
	const char* schemaName, 
	const char* tableName, 
	int position
)
{
	char* retval = NULL;
	char positionStr[10];
	mkDBConnection->errText = NULL;

	mkItoa(position, positionStr);

	DBInt_Statement* ociStatement = postgresqlCreateStatement(mkDBConnection);

	char* sql = mkStrcat(mkDBConnection->heapHandle, __FILE__, __LINE__,
							"SELECT "
							"	c.column_name, "
							"	c.data_type, "
							"	c.ordinal_position "
							"FROM "
							"	information_schema.table_constraints tc "
							"JOIN "
							"	information_schema.constraint_column_usage AS ccu USING(constraint_schema, constraint_name) "
							"JOIN "
							"	information_schema.columns AS c ON c.table_schema = tc.constraint_schema AND tc.table_name = c.table_name AND ccu.column_name = c.column_name "
							"WHERE "
							"		constraint_type = 'PRIMARY KEY' "
							"	and lower(c.table_schema) = lower('", schemaName, "') "
							"	and lower(tc.table_name) = lower('", tableName, "') "
							"	and c.ordinal_position = ", positionStr, " ",
							"order by "
							"	c.ordinal_position ",
							NULL
						);

	postgresqlPrepare(mkDBConnection, ociStatement, sql);

	postgresqlExecuteSelectStatement(mkDBConnection, ociStatement, sql);

	const char* tmp = postgresqlGetColumnValueByColumnName(mkDBConnection, ociStatement, "COLUMN_NAME");

	if (tmp) {
		retval = mkStrdup(mkDBConnection->heapHandle, tmp, __FILE__, __LINE__);
	}
	return retval;
}

POSTGRESQLINTERFACE_API char * postgresqlGetDatabaseName(DBInt_Connection * mkConnection) {
	PRECHECK(mkConnection);
	
	char *retval = NULL;
	if (mkConnection->connection.postgresqlHandle) {
		retval = PQdb(mkConnection->connection.postgresqlHandle);
		if (retval) {
			retval = mkStrdup(mkConnection->heapHandle, retval, __FILE__, __LINE__);
		}
	}

	POSTCHECK(mkConnection);
	return retval;
}

POSTGRESQLINTERFACE_API BOOL postgresqlRollback(DBInt_Connection *mkConnection) {
	PRECHECK(mkConnection);

	mkConnection->errText = NULL;
	PGresult *res = PQexec(mkConnection->connection.postgresqlHandle, "rollback");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
	}
	PQclear(res);
	
	// starting a new transaction
	if (mkConnection->errText == NULL) {
		beginNewTransaction(mkConnection);
	}
	POSTCHECK(mkConnection);
	// on success returns true
	return (mkConnection->errText != NULL);
}

POSTGRESQLINTERFACE_API BOOL postgresqlCommit(DBInt_Connection *mkConnection) {
	PRECHECK(mkConnection);

	mkConnection->errText = NULL;
	PGresult *res = PQexec(mkConnection->connection.postgresqlHandle, "commit");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
	}
	PQclear(res);
	
	// starting a new transaction
	if (mkConnection->errText == NULL) {
		beginNewTransaction(mkConnection);
	}
	POSTCHECK(mkConnection);
	// on success returns true
	return (mkConnection->errText != NULL);
}

POSTGRESQLINTERFACE_API void postgresqlInitConnection(DBInt_Connection * mkConnection) {
	PRECHECK(mkConnection);

	int ret = PQsetClientEncoding(mkConnection->connection.postgresqlHandle, "UNICODE");
	if (ret != 0) {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
	}

	POSTCHECK(mkConnection);
}


POSTGRESQLINTERFACE_API 
DBInt_Connection * 
postgresqlCreateConnection(
	HANDLE heapHandle, 
	DBInt_SupportedDatabaseType dbType, 
	const char * hostName,
	const char * dbName, 
	const char * userName, 
	const char * password
)
{
	DBInt_Connection *mkConnection = (DBInt_Connection *)mkMalloc(heapHandle, sizeof(DBInt_Connection), __FILE__, __LINE__);
	mkConnection->dbType = SODIUM_POSTGRESQL_SUPPORT;
	mkConnection->errText = NULL;
	mkConnection->heapHandle = heapHandle;
	mkConnection->err = FALSE;

	strcpy_s(mkConnection->hostName, HOST_NAME_LENGTH, hostName);

	if (IsPostgreSqLClientDriverLoaded == TRUE)
	{
		char* conninfo = mkStrcat(heapHandle, __FILE__, __LINE__, "host=", mkConnection->hostName, " dbname=", dbName, " user=", userName, " password=", password, NULL);
		PGconn* conn = PQconnectdb(conninfo);
		/* Check to see that the backend connection was successfully made */
		if (PQstatus(conn) != CONNECTION_OK) {
			mkConnection->err = TRUE;
			mkConnection->errText = PQerrorMessage(conn);
		}
		else {
			mkConnection->connection.postgresqlHandle = conn;
			// starting a new transaction
			beginNewTransaction(mkConnection);
		}

		mkFree(heapHandle, conninfo);
	}
	else 
	{
		mkConnection->err = TRUE;
		mkConnection->errText = "PostgreSql client driver not loaded";
	}
	
	return mkConnection;
}

POSTGRESQLINTERFACE_API void	postgresqlFreeStatement(DBInt_Connection *mkConnection, DBInt_Statement *stm) {
	PRECHECK(mkConnection);

	if (stm == NULL || mkConnection == NULL) {
		return;
	}
	
	if (stm->statement.postgresql.bindVariableCount > 0) {
		// Time to free all arrays
		mkFree(mkConnection->heapHandle, stm->statement.postgresql.bindVariables);
		stm->statement.postgresql.bindVariables = NULL;

		mkFree(mkConnection->heapHandle, stm->statement.postgresql.paramTypes);
		stm->statement.postgresql.paramTypes = NULL;

		mkFree(mkConnection->heapHandle, stm->statement.postgresql.paramFormats);
		stm->statement.postgresql.paramFormats = NULL;

		mkFree(mkConnection->heapHandle, stm->statement.postgresql.paramSizes);
		stm->statement.postgresql.paramSizes = NULL;

		stm->statement.postgresql.bindVariableCount = 0;
	}
	if (stm->statement.postgresql.resultSet) {
		PQclear(stm->statement.postgresql.resultSet);
		stm->statement.postgresql.resultSet = NULL;
	}
	mkFree(mkConnection->heapHandle, stm);
}

POSTGRESQLINTERFACE_API void postgresqlDestroyConnection(DBInt_Connection *mkConnection) {
	PRECHECK(mkConnection);

	if (mkConnection) {
		/* close the connection to the database and cleanup */
		PQfinish(mkConnection->connection.postgresqlHandle);
		mkConnection->connection.postgresqlHandle = NULL;
	}
}

POSTGRESQLINTERFACE_API void postgresqlBindLob(DBInt_Connection *mkConnection, DBInt_Statement *stm, const char *imageFileName, char *bindVariableName) {
	PRECHECK(mkConnection);

	if (mkConnection == NULL || imageFileName == NULL) {
		mkConnection->errText = "Function parameter(s) incorrect";
	}

	mkConnection->errText = NULL;

	// Import file content as large object
	Oid oid = writeLobContent(mkConnection, imageFileName);
	if (oid > 0) {
		// success. Adding it into bind parameter list
		char oidStr[15];// = (char*)mkMalloc(mkConnection->heapHandle, 15, __FILE__, __LINE__);
		//mkItoa(oid, );
		sprintf_s(oidStr, 15, "%d", oid);
		postgresqlAddNewParamEntry(mkConnection, stm, bindVariableName, 0, 0, oidStr, strlen(oidStr));
		//mkFree(mkConnection->heapHandle, oidStr);
	}
	else {
		mkConnection->errText = "Bind is unsuccessful";
		postgresqlAddNewParamEntry(mkConnection, stm, bindVariableName, 0, 0, "-1", 2);
	}
	POSTCHECK(mkConnection);
}

POSTGRESQLINTERFACE_API void *postgresqlGetLob(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName, DWORD *sizeOfValue) {
	PRECHECK(mkConnection);

	void *retVal = NULL;
	mkConnection->errText = NULL;
	HTSQL_COLUMN_TYPE colType = postgresqlGetColumnType(mkConnection, stm, columnName);

	// OID can be stored in INTEGER column
	if (colType == HTSQL_COLUMN_TYPE_NUMBER) {
		// getting column index by name
		int colNum = PQfnumber(stm->statement.postgresql.resultSet, columnName);
		if (stm->statement.postgresql.currentRowNum == -1) {
			stm->statement.postgresql.currentRowNum = 0;
		}
		
			// getting lob id
			char *lobOid = PQgetvalue(stm->statement.postgresql.resultSet, stm->statement.postgresql.currentRowNum, colNum);
			if (lobOid) {
				// getting actual blob content
				Oid lobId = atoi(lobOid);
				if (lobId != InvalidOid) {
					retVal = getLobContent(mkConnection, lobId, sizeOfValue);
				}
			}

		// returning blob binary content. return value should be FREE by caller 
		return retVal;
	}
	else if (colType == HTSQL_COLUMN_TYPE_LOB) {

		*sizeOfValue = postgresqlGetColumnSize(mkConnection, stm, columnName);
		const void *a = postgresqlGetColumnValueByColumnName(mkConnection, stm, columnName);
		return (void*)a;

	} else {
		mkConnection->errText = "Not a lob. Use appropriate function for that column or change the type of column to integer";
		return NULL;
	}
	POSTCHECK(mkConnection);
}

POSTGRESQLINTERFACE_API char *postgresqlExecuteInsertStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql) {
	PRECHECK(mkConnection);

	PGresult   *res = NULL;
	size_t	sizeOfRetval = 15;
	char *retval = mkMalloc(mkConnection->heapHandle, sizeOfRetval, __FILE__, __LINE__);
	mkItoa(0, retval);

	mkConnection->errText = NULL;

	if (stm->statement.postgresql.bindVariableCount == 0) {
		// "insert" statement has no bind variables
		res = PQexec(mkConnection->connection.postgresqlHandle, sql);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
			PQclear(res);
		}
		else {
			stm->statement.postgresql.currentRowNum = 0;
			stm->statement.postgresql.resultSet = res;
			char *rowCountAffected = PQcmdTuples(res);
			strcpy_s(retval, sizeOfRetval, rowCountAffected);
			//retval = postgresqlGetColumnValueByColumnName(mkConnection, stm, "oid");
		}
	}
	else {
		// "insert" statement has bind variables
		res = PQexecParams(mkConnection->connection.postgresqlHandle,
								sql,
								stm->statement.postgresql.bindVariableCount,
								stm->statement.postgresql.paramTypes,
								(const char * const*)stm->statement.postgresql.bindVariables,
								stm->statement.postgresql.paramSizes,
								stm->statement.postgresql.paramFormats,
								0);
		ExecStatusType statusType = PQresultStatus(res);
		if (statusType != PGRES_TUPLES_OK && statusType != PGRES_COMMAND_OK) {
			mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
			PQclear(res);
		}
		else {
			stm->statement.postgresql.currentRowNum = 0;
			stm->statement.postgresql.resultSet = res;
			char* rowCountAffected = PQcmdTuples(res);
			strcpy_s(retval, sizeOfRetval, rowCountAffected);
			//retval = postgresqlGetColumnValueByColumnName(mkConnection, stm, "oid");
		}
	}

	POSTCHECK(mkConnection);
	return retval;
}

POSTGRESQLINTERFACE_API void postgresqlExecuteUpdateStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql) {
	PRECHECK(mkConnection);

	PGresult   *res = NULL;
	mkConnection->errText = NULL;

	if (stm->statement.postgresql.bindVariableCount == 0) {
		// "update" statement has no bind variables
		res = PQexec(mkConnection->connection.postgresqlHandle, sql);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		}
	}
	else {
		// "update" statement has bind variables
		res = PQexecParams(mkConnection->connection.postgresqlHandle,
			sql,
			stm->statement.postgresql.bindVariableCount,
			stm->statement.postgresql.paramTypes,
			(const char * const*)stm->statement.postgresql.bindVariables,
			stm->statement.postgresql.paramSizes,
			stm->statement.postgresql.paramFormats,
			0);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		}
		else {
			stm->statement.postgresql.currentRowNum = 0;
			stm->statement.postgresql.resultSet = res;
		}
	}
	PQclear(res);
	stm->statement.postgresql.currentRowNum = 0;
	stm->statement.postgresql.resultSet = NULL;

	POSTCHECK(mkConnection);
}

POSTGRESQLINTERFACE_API void postgresqlBindString(DBInt_Connection *mkConnection, DBInt_Statement *stm, char *bindVariableName, char *bindVariableValue, size_t valueLength) {
	PRECHECK(mkConnection);

	if (bindVariableName && bindVariableValue) {
		postgresqlAddNewParamEntry(mkConnection,
			stm,
			bindVariableName,
			0 /* 0 means auto detection */,
			0 /* variable format */,
			bindVariableValue,
			valueLength);
	}

	POSTCHECK(mkConnection);
}

POSTGRESQLINTERFACE_API DBInt_Statement	*postgresqlCreateStatement(DBInt_Connection *mkConnection) {
	PRECHECK(mkConnection);

	DBInt_Statement	*retObj = (DBInt_Statement *)mkMalloc(mkConnection->heapHandle, sizeof(DBInt_Statement), __FILE__, __LINE__);
	retObj->statement.postgresql.resultSet = NULL;
	retObj->statement.postgresql.currentRowNum = 0;
	retObj->statement.postgresql.bindVariableCount = 0;
	retObj->statement.postgresql.bindVariables = NULL;
	return retObj;
}

POSTGRESQLINTERFACE_API const char *postgresqlGetColumnNameByIndex(DBInt_Connection *mkConnection, DBInt_Statement *stm, unsigned int index) {
	PRECHECK(mkConnection);

	const char *columnName = NULL;
	columnName = (const char *) PQfname(stm->statement.postgresql.resultSet, index);
	return columnName;
}

POSTGRESQLINTERFACE_API const char *postgresqlGetColumnValueByColumnName(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName) {
	PRECHECK(mkConnection);
	char* retVal = "";

	int colNum = PQfnumber(stm->statement.postgresql.resultSet, columnName);
	if (colNum < 0) 
	{
		// Column number not found
	} 
	else 
	{
		if (stm->statement.postgresql.currentRowNum == -1) {
			stm->statement.postgresql.currentRowNum = 0;
		}
		retVal = PQgetvalue(stm->statement.postgresql.resultSet, stm->statement.postgresql.currentRowNum, colNum);
	}
	return retVal;
}


POSTGRESQLINTERFACE_API unsigned int postgresqlGetColumnSize(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName) {
	PRECHECK(mkConnection);

	mkConnection->errText = NULL;
	int colNum = PQfnumber(stm->statement.postgresql.resultSet, columnName);
	unsigned int colSize = PQfsize(stm->statement.postgresql.resultSet, colNum);
	return colSize;
}

POSTGRESQLINTERFACE_API HTSQL_COLUMN_TYPE	postgresqlGetColumnType(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName) {
	PRECHECK(mkConnection);

	mkConnection->errText = NULL; 
	HTSQL_COLUMN_TYPE retVal = HTSQL_COLUMN_TYPE_NOTSET;
	int colNum = PQfnumber(stm->statement.postgresql.resultSet, columnName);
	Oid ctype = PQftype(stm->statement.postgresql.resultSet, colNum);
	switch (ctype) {
		case TIMESTAMPTZOID	:
		case ABSTIMEOID		:
		case TIMESTAMPOID:
		case DATEOID: {
			retVal = HTSQL_COLUMN_TYPE_DATE;
			break;
		}
		case CHAROID:
		case TEXTOID: {
			retVal = HTSQL_COLUMN_TYPE_TEXT;
			break;
		}
		case INT8OID:
		case INT2OID:
		case NUMERICOID:
		case INT4OID: {
			retVal = HTSQL_COLUMN_TYPE_NUMBER;
			break;
		}
		case OIDOID:
		case BYTEAOID: {
			retVal = HTSQL_COLUMN_TYPE_LOB;
			break;
		}
	}
	return retVal;
}

POSTGRESQLINTERFACE_API void postgresqlPrepare(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql) {
	PRECHECK(mkConnection);

	mkConnection->errText = NULL;
}


POSTGRESQLINTERFACE_API void postgresqlExecuteDescribe(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql) {
	PRECHECK(mkConnection); 
	
	mkConnection->errText = NULL;
	PGresult   *res = PQdescribePrepared(mkConnection->connection.postgresqlHandle, sql);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		PQclear(res);
	}
	else {
		stm->statement.postgresql.currentRowNum = 0;
		stm->statement.postgresql.resultSet = res;
	}
	POSTCHECK(mkConnection);
}


POSTGRESQLINTERFACE_API void postgresqlExecuteSelectStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql) {
	PRECHECK(mkConnection);

	mkConnection->errText = NULL;
	PGresult   *res = PQexec(mkConnection->connection.postgresqlHandle, sql);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		PQclear(res);
	}
	else {
		stm->statement.postgresql.currentRowNum = 0;
		stm->statement.postgresql.resultSet = res;
	}
	POSTCHECK(mkConnection);
}

POSTGRESQLINTERFACE_API void postgresqlRegisterString(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *bindVariableName, int maxLength) {
	PRECHECK(mkConnection);
	mkConnection->errText = NULL;
}

POSTGRESQLINTERFACE_API void postgresqlExecuteDeleteStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql) {
	PRECHECK(mkConnection); 
	
	mkConnection->errText = NULL;
	PGresult   *res = PQexec(mkConnection->connection.postgresqlHandle, sql);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
	}
	PQclear(res);
	stm->statement.postgresql.resultSet = NULL;
	stm->statement.postgresql.currentRowNum = 0;
	POSTCHECK(mkConnection);
}

POSTGRESQLINTERFACE_API void postgresqlExecuteAnonymousBlock(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql) {
	PRECHECK(mkConnection);

	mkConnection->errText = NULL;
	PGresult   *res = PQexec(mkConnection->connection.postgresqlHandle, sql);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
	}
	PQclear(res);
	stm->statement.postgresql.resultSet = NULL;
	stm->statement.postgresql.currentRowNum = 0;
	POSTCHECK(mkConnection);
}

POSTGRESQLINTERFACE_API unsigned int postgresqlGetAffectedRows(DBInt_Connection *mkConnection, DBInt_Statement *stm) {
	PRECHECK(mkConnection);

	return 0;
}

POSTGRESQLINTERFACE_API unsigned int postgresqlGetColumnCount(DBInt_Connection *mkConnection, DBInt_Statement *stm) {
	PRECHECK(mkConnection); 
	
	unsigned int colCount = 0;
	colCount = PQnfields(stm->statement.postgresql.resultSet);
	return colCount;
}

POSTGRESQLINTERFACE_API const char *postgresqlGetLastErrorText(DBInt_Connection *mkConnection) {
	PRECHECK(mkConnection); 
	return mkConnection->errText;
}

POSTGRESQLINTERFACE_API int postgresqlGetLastError(DBInt_Connection *mkConnection) {
	return 0;
}

POSTGRESQLINTERFACE_API void postgresqlSeek(DBInt_Connection *mkConnection, DBInt_Statement *stm, int rowNum) {
	PRECHECK(mkConnection);
	if (rowNum > 0) {
		stm->statement.postgresql.currentRowNum = rowNum - 1;
	} 	
}

POSTGRESQLINTERFACE_API BOOL	postgresqlNext(DBInt_Statement *stm) {
	stm->statement.postgresql.currentRowNum++;
	return postgresqlIsEof(stm);
}

POSTGRESQLINTERFACE_API BOOL	postgresqlIsEof(DBInt_Statement *stm) {
	if (stm == NULL || stm->statement.postgresql.resultSet == NULL) {
		return TRUE;
	}
	int nor = PQntuples(stm->statement.postgresql.resultSet);
	return (nor == 0 || stm->statement.postgresql.currentRowNum >= nor);
}

POSTGRESQLINTERFACE_API void	postgresqlFirst(DBInt_Statement *stm) {
	stm->statement.postgresql.currentRowNum = 0;
}

POSTGRESQLINTERFACE_API void	postgresqlLast(DBInt_Statement *stm) {
	int nor = PQntuples(stm->statement.postgresql.resultSet);
	stm->statement.postgresql.currentRowNum = nor-1;
}

POSTGRESQLINTERFACE_API int	postgresqlPrev(DBInt_Statement *stm) {
	if (stm->statement.postgresql.currentRowNum > 0) {
		stm->statement.postgresql.currentRowNum--;
	}
	return (stm->statement.postgresql.currentRowNum == 0);
}

POSTGRESQLINTERFACE_API int postgresqlIsConnectionOpen(DBInt_Connection * mkConnection) {
	PRECHECK(mkConnection);

	BOOL retval = FALSE;
	mkConnection->errText = NULL;
	PGresult   *res = PQexec(mkConnection->connection.postgresqlHandle, "");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mkConnection->errText = PQerrorMessage(mkConnection->connection.postgresqlHandle);
		retval = (mkConnection->errText == NULL || mkConnection->errText[0] == '\0');
	}
	PQclear(res);
	POSTCHECK(mkConnection);
	return retval;
}


