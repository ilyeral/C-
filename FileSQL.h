#include <iostream>
#include <string>
#include <list>
#include <cassert>

#include "windows.h"
#include "sql.h"
#include "sqlext.h"
#include "sqltypes.h"


class FileSQL {
public:
	SQLHENV henv = SQL_NULL_HENV;
	SQLHDBC hdbc = SQL_NULL_HDBC;
	SQLRETURN ret;
	SQLWCHAR dsn[64] = L"ems_data";
	SQLWCHAR user[64] = L"scada";
	SQLWCHAR password[64] = L"scadaems";

	FileSQL();
	void Open();
	void Close();

	bool Insert(char *filePath);
	bool Update();
	bool Delete();
	bool Select();

	void GetHstmtErrorMsg(SQLHSTMT hstmt);



};
