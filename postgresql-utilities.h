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