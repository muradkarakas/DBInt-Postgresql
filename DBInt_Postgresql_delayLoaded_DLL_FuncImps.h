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