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