#ifndef _POSTGRESQL_INTERFACE_H
#define _POSTGRESQL_INTERFACE_H

#define POSTGRESQLINTERFACE_API __declspec(dllexport)



// these are the column types and are orginally defined in include\server\catalog\pg_type.h
#define BOOLOID			16
#define BYTEAOID		17
#define CHAROID			18
#define NAMEOID			19
#define INT8OID			20
#define INT2OID			21
#define INT2VECTOROID	22
#define INT4OID			23
#define REGPROCOID		24
#define TEXTOID			25
#define OIDOID			26
#define TIDOID			27
#define XIDOID			28
#define CIDOID			29
#define OIDVECTOROID	30

#define TIMESTAMPTZOID	1184
#define ABSTIMEOID		702
#define TIMESTAMPOID	1114
#define DATEOID			1082
#define NUMERICOID		1700

#define PRECHECK(mkConnection)	{	if (mkConnection->errText) {	endTransaction(mkConnection); beginNewTransaction(mkConnection);	} }
#define POSTCHECK(mkConnection)	{	if (mkConnection->errText) {		} }


#ifdef __cplusplus
extern "C" {
#endif

	/* DDL's PUBLIC FUNCTIONS  */
	POSTGRESQLINTERFACE_API void				postgresqlInitConnection(DBInt_Connection * mkConnection);
	POSTGRESQLINTERFACE_API DBInt_Connection  * postgresqlCreateConnection(HANDLE heapHandle, DBInt_SupportedDatabaseType dbType, const char* hostName, const char *dbName, const char* userName, const char *password);
	POSTGRESQLINTERFACE_API void				postgresqlDestroyConnection(DBInt_Connection *mkConnection);
	POSTGRESQLINTERFACE_API int					postgresqlIsConnectionOpen(DBInt_Connection *mkConnection);
	POSTGRESQLINTERFACE_API const char			*postgresqlGetColumnValueByColumnName(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName);
	POSTGRESQLINTERFACE_API BOOL				postgresqlIsEof(DBInt_Statement *stm);
	POSTGRESQLINTERFACE_API void				postgresqlFirst(DBInt_Statement *stm);
	POSTGRESQLINTERFACE_API void				postgresqlLast(DBInt_Statement *stm);
	POSTGRESQLINTERFACE_API BOOL				postgresqlNext(DBInt_Statement *stm);
	POSTGRESQLINTERFACE_API int					postgresqlPrev(DBInt_Statement *stm);
	POSTGRESQLINTERFACE_API DBInt_Statement		*postgresqlCreateStatement(DBInt_Connection *mkConnection);
	POSTGRESQLINTERFACE_API void				postgresqlFreeStatement(DBInt_Connection *mkConnection, DBInt_Statement *stm);
	POSTGRESQLINTERFACE_API void				postgresqlSeek(DBInt_Connection *mkConnection, DBInt_Statement *stm, int rowNum);
	POSTGRESQLINTERFACE_API int					postgresqlGetLastError(DBInt_Connection *mkConnection);
	POSTGRESQLINTERFACE_API const char			*postgresqlGetLastErrorText(DBInt_Connection *mkConnection);
	POSTGRESQLINTERFACE_API BOOL				postgresqlCommit(DBInt_Connection *mkConnection);
	/*	CALLER MUST RELEASE RETURN VALUE  */
	POSTGRESQLINTERFACE_API char*				postgresqlGetPrimaryKeyColumn(DBInt_Connection* mkDBConnection, const char* schemaName, const char* tableName, int position);
	POSTGRESQLINTERFACE_API BOOL				postgresqlRollback(DBInt_Connection *mkConnection);
	POSTGRESQLINTERFACE_API void				postgresqlRegisterString(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *bindVariableName, int maxLength);
	POSTGRESQLINTERFACE_API unsigned int		postgresqlGetAffectedRows(DBInt_Connection *mkDBConnection, DBInt_Statement *stm);
	POSTGRESQLINTERFACE_API void				postgresqlBindString(DBInt_Connection *mkConnection, DBInt_Statement *stm, char *bindVariableName, char *bindVariableValue, size_t valueLength);
	POSTGRESQLINTERFACE_API void				postgresqlExecuteSelectStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql);
	POSTGRESQLINTERFACE_API void				postgresqlExecuteDescribe(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql);
	
	/* CALLER IS RESPONSIBLE TO RELEASE RETURN VALUE */
	POSTGRESQLINTERFACE_API char				*postgresqlExecuteInsertStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql);
	POSTGRESQLINTERFACE_API void				postgresqlExecuteDeleteStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql);
	POSTGRESQLINTERFACE_API void				postgresqlExecuteUpdateStatement(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql);
	POSTGRESQLINTERFACE_API void				postgresqlExecuteAnonymousBlock(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql);
	POSTGRESQLINTERFACE_API void				postgresqlPrepare(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *sql);
	POSTGRESQLINTERFACE_API unsigned int		postgresqlGetColumnCount(DBInt_Connection *mkDBConnection, DBInt_Statement *stm);

	POSTGRESQLINTERFACE_API HTSQL_COLUMN_TYPE	postgresqlGetColumnType(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName);
	POSTGRESQLINTERFACE_API unsigned int		postgresqlGetColumnSize(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName);

	/* return value must be free by caller*/
	POSTGRESQLINTERFACE_API void			  * postgresqlGetLob(DBInt_Connection * mkConnection, DBInt_Statement *stm, const char *columnName, DWORD *sizeOfValue);
	POSTGRESQLINTERFACE_API void				postgresqlBindLob(DBInt_Connection *mkConnection, DBInt_Statement *stm, const char *imageFileName, char *bindVariableName);
	POSTGRESQLINTERFACE_API const char		  * postgresqlGetColumnNameByIndex(DBInt_Connection *mkDBConnection, DBInt_Statement *stm, unsigned int index);
	
	/* return value must be free by caller*/
	POSTGRESQLINTERFACE_API char			  * postgresqlGetDatabaseName(DBInt_Connection * mkConnection);

#ifdef __cplusplus
}
#endif


#endif
