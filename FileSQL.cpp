#include"FileSQL.h"
#include <iostream>
#include <string>
#include <list>
#include <cassert>
#include <iomanip>
#include "windows.h"
#include "sql.h"
#include "sqlext.h"
#include "sqltypes.h"
#include <bitset>
#include <fstream>
#include <vector>
#include <iterator>
#include<thread>
#define MAX 16*10000
using namespace std;
#pragma comment(lib,"odbc32.lib")
void SplitString(vector<char>& vec, string str);
void SplitString(vector<char>& vec, string str1, string str2);
void SplitString(vector<char>& vec, string str1, string str2, string str3);
void SplitIntToHex(vector<char>& vec, int num);
char IntToHex(int num);
void CharTOSQLWCHAR(char* c, SQLWCHAR* s);
char HexToInt(char c1, char c2);
void SplitCharToBinary(vector<char>& vec, char c);
string ToString(int num);
int value[MAX];

FileSQL::FileSQL() {
	std::cout << "新建对象" << std::endl;
	henv = SQL_NULL_HENV;
	hdbc = SQL_NULL_HDBC;


	char* c_dsn = new char[64];
	string s_dsn = "ems_data"; 
	strcpy(c_dsn, s_dsn.c_str());
	dsn=new SQLWCHAR[64];
	MultiByteToWideChar(CP_UTF8, 0, c_dsn, strlen(c_dsn), dsn, 64);
	dsn[strlen(c_dsn)] = '\0';

	char* c_user = new char[64];
	string s_user = "scada";
	strcpy(c_user, s_user.c_str());
	user=new SQLWCHAR[64];
	MultiByteToWideChar(CP_UTF8, 0, c_user, strlen(c_user), user, 64);
	user[strlen(c_user)] = '\0';

	char* c_password = new char[64];
	string s_password = "scadaems";
	strcpy(c_password, s_password.c_str());
	password=new SQLWCHAR[64];
	MultiByteToWideChar(CP_UTF8, 0, c_password, strlen(c_password), password, 64);
	password[strlen(c_password)] = '\0';

}

FileSQL::FileSQL(string s_dsn = "ems_data", string s_user = "scada", string s_password = "scadaems") {
	std::cout << "新建对象" << std::endl;
	henv = SQL_NULL_HENV;
	hdbc = SQL_NULL_HDBC;


	char* c_dsn = new char[64];
	strcpy(c_dsn, s_dsn.c_str());
	dsn = new SQLWCHAR[64];
	MultiByteToWideChar(CP_UTF8, 0, c_dsn, strlen(c_dsn), dsn, 64);
	dsn[strlen(c_dsn)] = '\0';

	char* c_user = new char[64];
	strcpy(c_user, s_user.c_str());
	user = new SQLWCHAR[64];
	MultiByteToWideChar(CP_UTF8, 0, c_user, strlen(c_user), user, 64);
	user[strlen(c_user)] = '\0';

	char* c_password = new char[64];
	strcpy(c_password, s_password.c_str());
	password = new SQLWCHAR[64];
	MultiByteToWideChar(CP_UTF8, 0, c_password, strlen(c_password), password, 64);
	password[strlen(c_password)] = '\0';

}

void FileSQL::Open() {
	//std::cout << "open hdbc:" << hdbc << std::endl;

	// Allocate environment handle.
	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate environment handle!" << std::endl;
	}
	else {
		std::cout << "allocate environment handle!" << std::endl;
	}

	// Set the ODBC version environment attribute.
	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to set the ODBC version environment attribute!" << std::endl;
	}
	else {
		std::cout << "set the ODBC version environment attribute!" << std::endl;
	}

	// Allocate connection handle.
	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate connection handle!" << std::endl;
	}
	else {
		std::cout << "allocate connection handle!" << std::endl;
	}
	//std::cout << "open hdbc:" << hdbc << std::endl;

	// Connect the sqlserver.
	bool use_odbc_management = true;

	if (use_odbc_management) {  // (A)使用odbc管理器连接。
	  // DSN这个名字是odbc管理器中创建的DSN的名字，不是server或者数据库的名字。
	  // 所要访问的server及数据库在管理器中配置。

		ret = SQLConnect(hdbc, dsn, SQL_NTS, user, SQL_NTS, password, SQL_NTS);
		if (!SQL_SUCCEEDED(ret)) {
			std::cout << "Failed to connect the sqlserver!" << std::endl;
		}
		else {
			std::cout << "connect the sqlserver!" << std::endl;

		}
	}
	else {  // (B)使用字符串连接。
		SQLWCHAR out_string[1024];
		SQLSMALLINT out_len;
		SQLWCHAR dsn[256] = L"Driver={SQL Server};Server=CNSHWDW22;Database=test;Trusted_Connection=Yes;";
		ret = SQLDriverConnect(hdbc, NULL, dsn, SQL_NTS, out_string, 1024, &out_len, SQL_DRIVER_COMPLETE);;
	}
}

void FileSQL::Close() {
	//std::cout << "close hdbc:" << hdbc << std::endl;
	// Disconnect sqlserver.
	ret = SQLDisconnect(hdbc);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to disconnect sqlserver!" << std::endl;
	}

	// Release connection handle.
	ret = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release connection handle!" << std::endl;
	}

	// Release environment handle.
	ret = SQLFreeHandle(SQL_HANDLE_ENV, henv);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release environment handle!" << std::endl;
	}
}

bool FileSQL::Insert(char* filePath) {
	bool result = true;
	//文件处理
	ifstream ifs(filePath, ios::binary);
	ifs.seekg(0, ios::end);
	long len = ifs.tellg();
	ifs.seekg(0, ios::beg);
	std::vector<char> data;
	char c;
	int index = 0;
	while (index < len) {
		c = ifs.get();
		SplitIntToHex(data, c);
		//SplitCharToBinary(data, c);
		index++;
	}
	ifs.close();



	//数据库操作
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}
	char fileDrive[_MAX_DRIVE];
	char fileDir[_MAX_DIR];
	char fileName[_MAX_FNAME];
	char fileExt[_MAX_EXT];
	_splitpath(filePath, fileDrive, fileDir, fileName, fileExt);


	vector<char> sql;
	string s_sql = "insert into file_data (fileName,fileType,fileSize,data) values('";
	SplitString(sql, s_sql);
	string size = to_string(len);
	SplitString(sql, fileName);
	SplitString(sql, "','");
	SplitString(sql, fileExt);
	SplitString(sql, "',");
	SplitString(sql, size);
	SplitString(sql, ",0x");
	std::copy(data.begin(), data.end(), std::back_inserter(sql));
	SplitString(sql, ")");
	int s = sql.size();
	char* insert_sql = &sql[0];
	memset(insert_sql + s, '\0', 1);
	SQLWCHAR* _insert_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, insert_sql, strlen(insert_sql), _insert_sql, s);
	_insert_sql[strlen(insert_sql)] = '\0';



	ret = SQLPrepare(hstmt, (SQLWCHAR*)_insert_sql, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to prepare sql!" << std::endl;
		result = false;
	}


	std::cout << "SQL:" << insert_sql << std::endl;

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to execute sql!" << std::endl;
		GetHstmtErrorMsg(hstmt);
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}

	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}
	ifs.close();
	data.clear();
	data.swap(data);
	sql.clear();
	sql.swap(sql);
	insert_sql = NULL;
	return result;
}

bool FileSQL::Update(char* filePath) { 
	bool result = true;
	//文件处理
	ifstream ifs(filePath, ios::binary);
	ifs.seekg(0, ios::end);
	long len = ifs.tellg();
	ifs.seekg(0, ios::beg);
	std::vector<char> data;
	char c;
	int index = 0;
	while (index < len) {
		c = ifs.get();
		SplitIntToHex(data, c);
		index++;
	}
	ifs.close();



	//数据库操作
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}
	char fileDrive[_MAX_DRIVE];
	char fileDir[_MAX_DIR];
	char fileName[_MAX_FNAME];
	char fileExt[_MAX_EXT];
	_splitpath(filePath, fileDrive, fileDir, fileName, fileExt);


	vector<char> sql;
	string s_sql = "update file_data set data=";
	SplitString(sql, s_sql);
	SplitString(sql, "0x");
	std::copy(data.begin(), data.end(), std::back_inserter(sql));
	SplitString(sql, " where fileName='");
	SplitString(sql, fileName);
	SplitString(sql, "' and fileType='");
	SplitString(sql, fileExt);
	SplitString(sql, "'");


	int s = sql.size();
	char* update_sql = &sql[0];
	memset(update_sql + s, '\0', 1);
	SQLWCHAR* _update_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, update_sql, strlen(update_sql), _update_sql, s);
	_update_sql[strlen(update_sql)] = '\0';



	ret = SQLPrepare(hstmt, (SQLWCHAR*)_update_sql, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to prepare sql!" << std::endl;
		result = false;
	}


	//std::cout << "SQL:" << insert_sql << std::endl;

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to execute sql!" << std::endl;
		GetHstmtErrorMsg(hstmt);
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}
	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}
	ifs.close();
	data.clear();
	data.swap(data);
	sql.clear();
	sql.swap(sql);
	update_sql = NULL;
	return result;
}

bool FileSQL::Delete(char* id) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;
	string s_sql = "delete from file_data where id=";
	SplitString(sql, s_sql);
	SplitString(sql, id);

	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	//cout << insert_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}

bool FileSQL::SelectAll() {
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	SQLCHAR name[50] = { 0 };
	SQLCHAR type[50] = { 0 };
	long len = 0;
	std::vector<BYTE> data;

	SQLWCHAR select_sql[256] = L"select id,fileName,fileType,fileSize from file_data";
	ret = SQLExecDirect(hstmt, select_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
	}
	SQLINTEGER cbsatid = SQL_NTS;
	cout << setw(5) << left << "id";
	cout << setw(5) << left << "name";
	cout << setw(5) << left << "type";
	cout << setw(5) << left << "size" << endl;
	while (SQLFetch(hstmt) != SQL_NO_DATA_FOUND)
	{
		SDWORD id;
		SQLCHAR name[50];
		SQLCHAR type[10];
		SDWORD size;

		SQLGetData(hstmt, 1, SQL_C_LONG, (int*)& id, sizeof(id), &cbsatid);
		SQLGetData(hstmt, 2, SQL_C_CHAR, name, 50, &cbsatid);
		SQLGetData(hstmt, 3, SQL_C_CHAR, type, 10, &cbsatid);
		SQLGetData(hstmt, 4, SQL_C_LONG, (int*)& size, sizeof(size), &cbsatid);

		cout << setw(5) << left << id;
		cout << setw(5) << left << name;
		cout << setw(5) << left << type;
		cout << setw(5) << left << size << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	data.clear();
	data.swap(data);
	return true;
}

bool FileSQL::SelectById(char* id) {
	bool result = true;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	SQLCHAR name[50] = { 0 };
	SQLCHAR type[50] = { 0 };
	long len = 0;

	vector<char> sql;
	string s_sql = "select fileName,fileType,fileSize,data from file_data where id=";
	SplitString(sql, s_sql);
	SplitString(sql, id);

	int s = sql.size();
	char* select_sql = &sql[0];
	memset(select_sql + s, '\0', 1);
	SQLWCHAR* _select_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, select_sql, strlen(select_sql), _select_sql, s);
	_select_sql[strlen(select_sql)] = '\0';

	//cout << insert_sql << endl;

	ret = SQLExecDirect(hstmt, _select_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	SQLINTEGER cbsatid = SQL_NTS;

	while (SQLFetch(hstmt) != SQL_NO_DATA_FOUND)
	{
		SQLCHAR name[50];
		SQLCHAR type[10];
		SDWORD size;

		SQLGetData(hstmt, 1, SQL_C_CHAR, name, 50, &cbsatid);
		SQLGetData(hstmt, 2, SQL_C_CHAR, type, 10, &cbsatid);
		SQLGetData(hstmt, 3, SQL_C_LONG, (int*)& size, sizeof(size), &cbsatid);
		char* data = new char[2 * size+1];
		SQLGetData(hstmt, 4, SQL_C_CHAR, data, 2 * size+2, &cbsatid);
		char* file_data = new char[size + 1];
		for (int i = 0; i < size; i++) {
			file_data[i] = HexToInt(data[i * 2], data[i * 2 + 1]);
		}
		vector<int> v_file_data(file_data, file_data + size);
		ofstream ofs(strcat((char*)& name, (char*)& type), std::ios::out | std::ios::binary);
		copy(v_file_data.begin(), v_file_data.end(), ostream_iterator<char>(ofs));
		ofs.close();

		v_file_data.clear();
		v_file_data.swap(v_file_data);
		delete[] data;
		delete[] file_data;

	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	sql.clear();
	sql.swap(sql);
	return result;
}



bool FileSQL::TableExistence() {
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}


	vector<char> sql;



	//SplitString(sql, "USE [SCADA_DATA]\n");

	SplitString(sql, "if object_id(N'file_data',N'U') is null\n");
	SplitString(sql, "Begin\n");

	SplitString(sql, "SET ANSI_NULLS ON\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "SET QUOTED_IDENTIFIER ON\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "SET ANSI_PADDING ON\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "CREATE TABLE [dbo].[file_data](\n");
	SplitString(sql, "[id] [int] IDENTITY(1,1) NOT NULL,\n");
	SplitString(sql, "[fileName] [nvarchar](50) NOT NULL,\n");
	SplitString(sql, "[fileType] [varchar](50) NOT NULL,\n");
	SplitString(sql, "[uploader] [nvarchar](50) NULL,\n");
	SplitString(sql, "[uploadTime] [datetime] NULL,\n");
	SplitString(sql, "[fileSize] [bigint] NOT NULL,\n");
	SplitString(sql, "[data] [varbinary](max) NULL,\n");
	SplitString(sql, " CONSTRAINT [PK_file_data] PRIMARY KEY CLUSTERED \n");
	SplitString(sql, "(\n");
	SplitString(sql, "[id] ASC\n");
	SplitString(sql, ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]\n");
	SplitString(sql, ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "SET ANSI_PADDING OFF\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "End\n");

	int s = sql.size();
	char* insert_sql = &sql[0];
	memset(insert_sql + s, '\0', 1);
	SQLWCHAR* _insert_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, insert_sql, strlen(insert_sql), _insert_sql, s);
	_insert_sql[strlen(insert_sql)] = '\0';

	ret = SQLPrepare(hstmt, (SQLWCHAR*)_insert_sql, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to prepare sql!" << std::endl;
	}

	//std::cout << "SQL:" << insert_sql << std::endl;

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to execute sql!" << std::endl;
		GetHstmtErrorMsg(hstmt);
	}
	else {
		cout << "操作成功" << endl;
	}
	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	sql.clear();
	sql.swap(sql);
	insert_sql = NULL;
	return true;
}
bool FileSQL::ScriptTableExistence() {
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}


	vector<char> sql;



	//SplitString(sql, "USE [SCADA_DATA]\n");

	SplitString(sql, "if object_id(N'file_data_script',N'U') is null\n");
	SplitString(sql, "Begin\n");

	SplitString(sql, "SET ANSI_NULLS ON\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "SET QUOTED_IDENTIFIER ON\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "SET ANSI_PADDING ON\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "CREATE TABLE [dbo].[file_data_script](\n");
	SplitString(sql, "[id] [int] IDENTITY(1,1) NOT NULL,\n");
	SplitString(sql, "[fileName] [nvarchar](50) NOT NULL,\n");
	SplitString(sql, "[fileType] [varchar](50) NOT NULL,\n");
	SplitString(sql, "[uploader] [nvarchar](50) NULL,\n");
	SplitString(sql, "[uploadTime] [datetime] NULL,\n");
	SplitString(sql, "[fileSize] [bigint] NOT NULL,\n");
	SplitString(sql, "[data] [varbinary](max) NULL,\n");
	SplitString(sql, "[srvr] [nvarchar](50) NOT NULL,\n");
	SplitString(sql, "[operation] [int] NOT NULL,\n");
	SplitString(sql, "[dbName] [nvarchar](50) NOT NULL,\n");
	SplitString(sql, " CONSTRAINT [PK_file_data_script] PRIMARY KEY CLUSTERED \n");
	SplitString(sql, "(\n");
	SplitString(sql, "[id] ASC\n");
	SplitString(sql, ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]\n");
	SplitString(sql, ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "SET ANSI_PADDING OFF\n");
	//SplitString(sql, "GO\n");
	SplitString(sql, "End\n");

	int s = sql.size();
	char* insert_sql = &sql[0];
	memset(insert_sql + s, '\0', 1);
	SQLWCHAR* _insert_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, insert_sql, strlen(insert_sql), _insert_sql, s);
	_insert_sql[strlen(insert_sql)] = '\0';

	ret = SQLPrepare(hstmt, (SQLWCHAR*)_insert_sql, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to prepare sql!" << std::endl;
	}

	//std::cout << "SQL:" << insert_sql << std::endl;

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to execute sql!" << std::endl;
		GetHstmtErrorMsg(hstmt);
	}
	else {
		cout << "操作成功" << endl;
	}
	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	sql.clear();
	sql.swap(sql);
	insert_sql = NULL;
	return true;
}


void FileSQL::GetHstmtErrorMsg(SQLHSTMT hstmt) {
	SQLWCHAR error_msg[1024] = { 0 };
	SQLSMALLINT msg_len = 0;
	SQLWCHAR sql_state[6] = { 0 };
	SQLINTEGER error_code = 0;

	int i = 1;
	while (true) {
		if (SQLGetDiagRecW(SQL_HANDLE_STMT, hstmt, i, sql_state, &error_code, error_msg, 1024, &msg_len) != SQL_NO_DATA) {
			std::wcout << error_msg << std::endl;
		}
		else {
			break;
		}
		i++;
	}
}



bool FileSQL::BindDataBase(string ip,string user,string password) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;

	string::size_type p;
	p = ip.find_last_of(".");
	string temp;
	temp.assign(ip, p + 1, ip.size());

	SplitString(sql, "exec sp_addlinkedserver'C");
	SplitString(sql, temp);
	SplitString(sql, "' ,'' , 'SQLOLEDB' ,'");
	SplitString(sql, ip);
	SplitString(sql, "'\n");
	SplitString(sql, "exec sp_addlinkedsrvlogin'C");
	SplitString(sql, temp);
	SplitString(sql, "' ,'false' ,null , '");
	SplitString(sql, user);
	SplitString(sql, "' ,'");
	SplitString(sql, password);
	SplitString(sql, "'");


	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}

bool FileSQL::UnBindDataBase(string name) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;
	string s_sql_1 = "exec sp_droplinkedsrvlogin'";
	string s_sql_2 = "exec sp_dropserver'";
	SplitString(sql, s_sql_1);
	SplitString(sql, name);
	SplitString(sql, "' ,null \n");
	SplitString(sql, s_sql_2);
	SplitString(sql, name);
	SplitString(sql, "'");


	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "关闭连接成功" << endl;
		ClearScriptBySrvr(name);
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}

bool FileSQL::SelectAllBind() {
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	SQLWCHAR select_sql[256] = L"exec sp_helpserver";
	ret = SQLExecDirect(hstmt, select_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
	}
	SQLINTEGER cbsatid = SQL_NTS;

	while (SQLFetch(hstmt) != SQL_NO_DATA_FOUND)
	{
		SQLCHAR name[50];



		SQLGetData(hstmt, 1, SQL_C_CHAR, name, 50, &cbsatid);


		cout << setw(5) << left << name;

	}
	cout << endl;

	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}
	return true;
}



void FileSQL::CreateTriggerForBind(string bindName, string dbName) {
	CreateI_TriggerForBind(bindName,  dbName);
	CreateD_TriggerForBind(bindName,  dbName);
	CreateU_TriggerForBind(bindName, dbName);
}

bool FileSQL::CreateI_TriggerForBind(string bindName,string dbName) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;

	SplitString(sql, "create trigger fileTriggerInsert", bindName," on file_data after insert as begin\n");
	SplitString(sql, "if exists (select * from inserted)\n");
	SplitString(sql, "begin\n");
	SplitString(sql, "declare @srvr nvarchar(128), @operation int,@dbName nvarchar(50);\n");
	SplitString(sql, "set @srvr = '", bindName,"';\n");
	SplitString(sql, "set @dbName = '", dbName, "';\n");
	SplitString(sql, "set @operation = 1;\n"); 
	SplitString(sql, "declare @a varchar(50),@b varchar(50),@c bigint,@d varbinary(max);\n");
	SplitString(sql, "select @a =fileName,@b= fileType,@c=fileSize,@d=data from inserted;\n");
	SplitString(sql, "if not exists (select * from file_data_script where fileName=@a and fileType=@b)\n");
	SplitString(sql, "insert into file_data_script(fileName,fileType,fileSize,data,srvr,operation,dbName)values(@a,@b,@c,@d,@srvr,@operation,@dbName);\n");
	SplitString(sql, "else\n");
	SplitString(sql, "update file_data_script set fileSize=@c,data=@d where fileName=@a and fileType= @b;\n");
	SplitString(sql, "end\n");
	SplitString(sql, "end\n");

	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "执行失败!" << std::endl;
		result = false;
	}
	else {
		cout << "执行成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}
	sql.clear();
	sql.swap(sql);
	return result;
}

bool FileSQL::CreateD_TriggerForBind(string bindName, string dbName) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;
	SplitString(sql, "create trigger fileTriggerDelete",bindName," on file_data after delete as begin\n");
	SplitString(sql, "if exists (select * from deleted)\n");
	SplitString(sql, "begin\n");
	SplitString(sql, "declare @srvr nvarchar(128),@operation int,@dbName nvarchar(50);\n");
	SplitString(sql, "set @srvr = '",bindName,"';\n");
	SplitString(sql, "set @dbName = '", dbName, "';\n");
	SplitString(sql, "set @operation = 2;\n");
	SplitString(sql, "declare @a varchar(50),@b varchar(50),@c bigint,@d varbinary(max);\n");
	SplitString(sql, "select @a =fileName,@b= fileType,@c=fileSize,@d=data from deleted;\n");
	SplitString(sql, "if not exists (select * from file_data_script where fileName=@a and fileType=@b)\n");
	SplitString(sql, "    insert into file_data_script(fileName,fileType,fileSize,data,srvr,operation,dbName)values(@a,@b,@c,@d,@srvr,@operation,@dbName); \n");
	SplitString(sql, "else\n");
	SplitString(sql, "    delete from file_data_script where fileName=@a and fileType=@b;\n");
	SplitString(sql, "end\n");
	SplitString(sql, "end\n");

	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "执行失败!" << std::endl;
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}

bool FileSQL::CreateU_TriggerForBind(string bindName, string dbName) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;

	SplitString(sql, "create trigger fileTriggerUpdate",bindName," on file_data after update as begin\n");
	SplitString(sql, "if exists (select * from deleted)\n");
	SplitString(sql, "begin\n");
	SplitString(sql, "declare @srvr nvarchar(128),@operation int,@dbName nvarchar(50);\n");
	SplitString(sql, "set @srvr = '",bindName,"';\n");
	SplitString(sql, "set @dbName = '", dbName, "';\n");
	SplitString(sql, "set @operation = 3;\n");
	SplitString(sql, "declare @a varchar(50),@b varchar(50),@c bigint,@d varbinary(max);\n");
	SplitString(sql, "declare @cc bigint,@dd varbinary(max);\n");
	SplitString(sql, "select @a =fileName,@b= fileType,@c=fileSize,@d=data from deleted;\n");
	SplitString(sql, "select @cc=fileSize,@dd=data from inserted;\n");
	SplitString(sql, "if not exists (select * from file_data_script where fileName=@a and fileType=@b)\n");
	SplitString(sql, "insert into file_data_script(fileName,fileType,fileSize,data,srvr,operation,dbName)values(@a,@b,@cc,@dd,@srvr,@operation,@dbName);\n");
	SplitString(sql, "else\n");
	SplitString(sql, "update file_data_script set fileSize=@cc,data=@dd where fileName=@a and fileType= @b;\n");
	SplitString(sql, "end\n");
	SplitString(sql, "end\n");

	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "执行失败!" << std::endl;
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}



void FileSQL::DropTriggerForBind(string bindName) {
	DropI_TriggerForBind(bindName);
	DropD_TriggerForBind(bindName);
	DropU_TriggerForBind(bindName);
}

bool FileSQL::DropI_TriggerForBind(string bindName) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;

	SplitString(sql, "drop trigger fileTriggerInsert");
	SplitString(sql, bindName);
	SplitString(sql, ";");


	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}

bool FileSQL::DropD_TriggerForBind(string bindName) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;
	SplitString(sql, "drop trigger fileTriggerDelete");
	SplitString(sql, bindName);
	SplitString(sql, ";");



	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}

bool FileSQL::DropU_TriggerForBind(string bindName) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;

	SplitString(sql, "drop trigger fileTriggerUpdate");
	SplitString(sql, bindName);
	SplitString(sql, ";");



	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	cout << delete_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "操作成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}




bool FileSQL::DoScript() {
	cout << "后台执行同步操作" << endl;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	SQLCHAR name[50] = { 0 };
	SQLCHAR type[50] = { 0 };
	long len = 0;
	std::vector<BYTE> data;

	SQLWCHAR select_sql[256] = L"select id,fileName,fileType,fileSize,data,srvr,operation,dbName from file_data_script";
	ret = SQLExecDirect(hstmt, select_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
	}
	SQLINTEGER cbsatid = SQL_NTS;
	while (SQLFetch(hstmt) != SQL_NO_DATA_FOUND)
	{
		int id;
		char name[50];
		char type[10];
		long size;
		char* data ;
		char srvr[50];
		int operation;
		char dbName[50];

		SQLGetData(hstmt, 1, SQL_C_LONG, (int*)& id, sizeof(id), &cbsatid);
		SQLGetData(hstmt, 2, SQL_C_CHAR, name, 50, &cbsatid);
		SQLGetData(hstmt, 3, SQL_C_CHAR, type, 10, &cbsatid);
		SQLGetData(hstmt, 4, SQL_C_LONG, (long*)& size, sizeof(size), &cbsatid);

		data = new char[size*3+1];

		SQLGetData(hstmt, 5, SQL_C_CHAR, data, size*3+1  , &cbsatid);
		SQLGetData(hstmt, 6, SQL_C_CHAR, srvr, 50, &cbsatid);
		SQLGetData(hstmt, 7, SQL_C_LONG, (int*)& operation, sizeof(operation), &cbsatid);
		SQLGetData(hstmt, 8, SQL_C_CHAR, dbName, 50, &cbsatid);
		cout << srvr<<"-" << operation <<"-"<< size;
		int r= ExecuteScript(id, name, type, size, data, srvr, operation, dbName);
		ClearScriptById(r);
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	data.clear();
	data.swap(data);
	return true;
}
int FileSQL::ExecuteScript(int id,char * fName,char *fType,long fSize,char *data,char *srvr,int operation,char * dbName) {
	int key = -1;
	//数据库操作
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return -1;
	}

	string size = to_string(fSize);

	vector<char> sql;
	switch (operation) {
	case 1://增

		//判断是否存在数据
		SplitString(sql, "if not exists (select * from ", srvr, ".");
		SplitString(sql, dbName, ".dbo.file_data where fileName='",fName);
		SplitString(sql, "' and fileType='", fType,"')\n");
		//执行
		SplitString(sql, "insert into ",srvr,".");
		SplitString(sql, dbName, ".dbo.file_data ");
		SplitString(sql, "(fileName,fileType,fileSize,data)values(");
		SplitString(sql, "'",fName, "',");
		SplitString(sql, "'", fType, "',");
		SplitString(sql, size, ",0x");
		SplitString(sql, data, ")");
		break;
	case 2://删
		SplitString(sql, "delete from ", srvr, ".");
		SplitString(sql, dbName, ".dbo.file_data ");
		SplitString(sql, "where fileName='",fName,"'");
		SplitString(sql, "and fileType='", fType,"'");
		SplitString(sql, "and fileSize=", size);
		break;
	case 3://改
		SplitString(sql, "update ", srvr, ".");
		SplitString(sql, dbName, ".dbo.file_data ");
		SplitString(sql, "set fileSize=",size,",");
		SplitString(sql, "data=", data);
		SplitString(sql, " where fileName='", fName, "' ");
		SplitString(sql, "and fileType='", fType, "' ");
		SplitString(sql, "and fileSize=", size, " ");
		SplitString(sql, "and data=", data, ";");
		break;
	}

	int s = sql.size();
	char* insert_sql = &sql[0];
	memset(insert_sql + s, '\0', 1);
	SQLWCHAR* _insert_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, insert_sql, strlen(insert_sql), _insert_sql, s);
	_insert_sql[strlen(insert_sql)] = '\0';



	ret = SQLPrepare(hstmt, (SQLWCHAR*)_insert_sql, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to prepare sql!" << std::endl;
		key = -1;
	}


	std::cout << "SQL:" << insert_sql << std::endl;

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS) {
		std::cout << "同步失败!" << std::endl;
		GetHstmtErrorMsg(hstmt);
		key = -1;
	}
	else {
		cout << "同步成功" << endl;
		key = id;
	}

	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}
	sql.clear();
	sql.swap(sql);
	insert_sql = NULL;
	return key;
}
bool FileSQL::ClearScriptById(int id) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;
	SplitString(sql, "delete from file_data_Script where id=", to_string(id));

	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	//cout << insert_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "删除脚本成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}
bool FileSQL::ClearScriptBySrvr(string srvr) {
	bool result = true;

	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	// Allocate execute statement handle.
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to allocate execute statement handle!" << std::endl;
		return false;
	}

	vector<char> sql;
	SplitString(sql, "delete from file_data_Script where srvr='", srvr,"'");

	int s = sql.size();
	char* delete_sql = &sql[0];
	memset(delete_sql + s, '\0', 1);
	SQLWCHAR* _delete_sql = new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, delete_sql, strlen(delete_sql), _delete_sql, s);
	_delete_sql[strlen(delete_sql)] = '\0';

	//cout << insert_sql << endl;

	ret = SQLExecDirect(hstmt, _delete_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
		result = false;
	}
	else {
		cout << "删除脚本成功" << endl;
	}


	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}

	return result;
}
void SplitString(vector<char>& vec, string str) {
	int len = str.length();
	const char* p = str.data();
	for (int i = 0; i < len; i++) {
		vec.push_back(p[i]);
	}
}
void SplitString(vector<char>& vec, string str1,string str2) {
	int len1 = str1.length();
	const char* p1 = str1.data();
	for (int i = 0; i < len1; i++) {
		vec.push_back(p1[i]);
	}
	int len2 = str2.length();
	const char* p2 = str2.data();
	for (int i = 0; i < len2; i++) {
		vec.push_back(p2[i]);
	}
}
void SplitString(vector<char>& vec, string str1,string str2,string str3) {
	int len1 = str1.length();
	const char* p1 = str1.data();
	for (int i = 0; i < len1; i++) {
		vec.push_back(p1[i]);
	}
	int len2 = str2.length();
	const char* p2 = str2.data();
	for (int i = 0; i < len2; i++) {
		vec.push_back(p2[i]);
	}
	int len3 = str3.length();
	const char* p3 = str3.data();
	for (int i = 0; i < len3; i++) {
		vec.push_back(p3[i]);
	}
}
void SplitIntToHex(vector<char>& vec, int num) {
	char str[2];
	if (num < 0) {
		num = -num;
		num += 128;
	}
	str[0] = IntToHex(num / 16);
	str[1] = IntToHex(num % 16);
	for (int i = 0; i < 2; i++) {
		vec.push_back(str[i]);
	}
}
char IntToHex(int num) {
	switch (num) {
	case 1:return '1'; break;
	case 2:return'2'; break;
	case 3:return'3'; break;
	case 4:return'4'; break;
	case 5:return'5'; break;
	case 6:return'6'; break;
	case 7:return'7'; break;
	case 8:return'8'; break;
	case 9:return'9'; break;
	case 10:return'A'; break;
	case 11:return'B'; break;
	case 12:return'C'; break;
	case 13:return'D'; break;
	case 14:return'E'; break;
	case 15:return'F'; break;
	case 0:return'0'; break;
	}
}
char HexToInt(char c1, char c2) {
	int num = 1;
	switch (c1) {
	case '1':num = num * 16 * 1; break;
	case '2':num = num * 16 * 2; break;
	case '3':num = num * 16 * 3; break;
	case '4':num = num * 16 * 4; break;
	case '5':num = num * 16 * 5; break;
	case '6':num = num * 16 * 6; break;
	case '7':num = num * 16 * 7; break;
	case '8':num = num * 16 * 8; break;
	case '9':num = num * 16 * 9; break;
	case 'A':num = num * 16 * 10; break;
	case 'B':num = num * 16 * 11; break;
	case 'C':num = num * 16 * 12; break;
	case 'D':num = num * 16 * 13; break;
	case 'E':num = num * 16 * 14; break;
	case 'F':num = num * 16 * 15; break;
	case '0':num = num * 16 * 0; break;
	}
	switch (c2) {
	case '1':num = num + 1; break;
	case '2':num = num + 2; break;
	case '3':num = num + 3; break;
	case '4':num = num + 4; break;
	case '5':num = num + 5; break;
	case '6':num = num + 6; break;
	case '7':num = num + 7; break;
	case '8':num = num + 8; break;
	case '9':num = num + 9; break;
	case 'A':num = num + 10; break;
	case 'B':num = num + 11; break;
	case 'C':num = num + 12; break;
	case 'D':num = num + 13; break;
	case 'E':num = num + 14; break;
	case 'F':num = num + 15; break;
	case '0':num = num + 0; break;
	}
	if (num > 128) {
		num -= 128;
		num = -num;
	}
	return num;
}
void CharTOSQLWCHAR(char* c, SQLWCHAR* s) {
	for (int i = 0; i < strlen(c); i++)
	{
		s[i] = c[i];
	}
	s[strlen(c)] = '\0';
}

void SplitCharToBinary(vector<char>& vec, char c)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (c & 0x80) vec.push_back('1'); else vec.push_back('0');
		c <<= 1;
	}
}
string ToString(int num) {
	int l = 0;
	while (num > 0) {
		l++;
		num /= 10;
	}
	char* ch = new char[l+1];
	sprintf(ch, "%d", num);
	return ch;
}
