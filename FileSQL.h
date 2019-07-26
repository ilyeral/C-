#include <iostream>
#include <string>
#include <list>
#include <cassert>

#include "windows.h"
#include "sql.h"
#include "sqlext.h"
#include "sqltypes.h"
using namespace std;

class FileSQL {
public:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLRETURN ret;
	SQLWCHAR *dsn;
	SQLWCHAR *user;
	SQLWCHAR *password;

	FileSQL();
	FileSQL(string s_dsn, string s_user, string s_password);
	void Open();
	void Close();

	bool Insert(char* filePath);
	bool Update(char* filePath);
	bool Delete(char* id);
	bool SelectById(char *id);
	bool SelectAll();

	bool TableExistence(); 
	bool ScriptTableExistence();
	bool BindDataBase(string ip, string user, string password);
	bool UnBindDataBase(string name);
	bool SelectAllBind();

	void CreateTriggerForBind(string bindName, string dbName);
	bool CreateI_TriggerForBind(string bindName, string dbName);
	bool CreateD_TriggerForBind(string bindName, string dbName);
	bool CreateU_TriggerForBind(string bindName, string dbName);


	void DropTriggerForBind(string bindName);
	bool DropI_TriggerForBind(string bindName);
	bool DropD_TriggerForBind(string bindName);
	bool DropU_TriggerForBind(string bindName);

	bool DoScript();
	int ExecuteScript(int id, char* fName, char* fType, long fSize, char* data, char* srvr, int operation, char* dbName);
	bool ClearScriptBySrvr(string srvr);
	bool ClearScriptById(int di);



	void GetHstmtErrorMsg(SQLHSTMT hstmt);
};
