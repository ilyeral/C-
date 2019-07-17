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
#define MAX 16*10000
using namespace std;
#pragma comment(lib,"odbc32.lib")
void SplitString(vector<char>& vec, string str);
void SplitInt(vector<char>& vec, int num);
char IntToHex(int num);
void CharTOSQLWCHAR(char* c, SQLWCHAR* s);
char HexToInt(char c1, char c2, char c3);
int value[MAX];

FileSQL::FileSQL() {
	std::cout << "新建对象" << std::endl;

}
void FileSQL::Open() {
	std::cout << "open hdbc:" << hdbc <<std::endl;

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
	}else{
		std::cout << "allocate connection handle!" << std::endl;
	}
	std::cout << "open hdbc:" << hdbc << std::endl;

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
	std::cout << "close hdbc:" << hdbc << std::endl;
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
bool FileSQL::Insert(char *filePath) {
	//文件处理
	ifstream ifs(filePath, ios::binary);
	ifs.seekg(0, ios::end);
	long len = ifs.tellg();
	ifs.seekg(0, ios::beg);
	std::vector<char> data;
	char c;
	int index = 0;
	while (index<len) {
		c = ifs.get();
		SplitInt(data, c);
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
	SplitString(sql,s_sql);
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
	memset(insert_sql+ s, '\0', 1);
	SQLWCHAR* _insert_sql =new SQLWCHAR[s];
	MultiByteToWideChar(CP_UTF8, 0, insert_sql, strlen(insert_sql), _insert_sql, s);
	_insert_sql[strlen(insert_sql)] = '\0';

	

	ret = SQLPrepare(hstmt, (SQLWCHAR*)_insert_sql, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to prepare sql!" << std::endl;
	}

	
	std::cout << "SQL:" << insert_sql<< std::endl;

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS) {
		std::cout << "Failed to execute sql!" << std::endl;
		GetHstmtErrorMsg(hstmt);
	}

	// Release execute statement handle.
	ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to release execute statement handle!" << std::endl;
	}
	//fclose(pFile);
	ifs.close();
	//delete[] data;
	data.clear();
	data.swap(data);
	sql.clear();
	sql.swap(sql);
	return true;
}
bool FileSQL::Update() { return false; }
bool FileSQL::Delete() { return false; }
bool FileSQL::Select() {
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

	SQLWCHAR select_sql[100256] = L"select fileName,fileType,fileSize,data from file_data where id=1052";
	ret = SQLExecDirect(hstmt, select_sql, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		std::cout << "Failed to query data!" << std::endl;
	}
	SQLINTEGER cbsatid = SQL_NTS;

	while (SQLFetch(hstmt) != SQL_NO_DATA_FOUND)
	{
		SQLCHAR name[50];
		SQLCHAR type[10];
		SDWORD size;

		SQLGetData(hstmt, 1, SQL_C_CHAR, name, 50, &cbsatid);
		SQLGetData(hstmt, 2, SQL_C_CHAR, type, 10, &cbsatid);
		SQLGetData(hstmt, 3, SQL_C_LONG, (int *)&size, sizeof(size), &cbsatid);
		char* data = new char[3 * size+3];
		SQLGetData(hstmt, 4, SQL_C_CHAR, data, 3*size+3, &cbsatid);
		char* file_data = new char[size+1];
		int n = size % 2 == 0 ? 0 : 1;
		for (int i = 0; i < size; i++) {
			file_data[i] = HexToInt(data[i*3+n],data[i*3+1+n],data[i*3+2+n]);
		}
		vector<int> v_file_data(file_data, file_data + size);
		ofstream ofs(strcat((char*)&name, (char*)& type) , std::ios::out | std::ios::binary);
		copy(v_file_data.begin(), v_file_data.end(), ostream_iterator<char>(ofs));
		ofs.close();
		delete[] data;
		delete[] file_data;
		v_file_data.clear();
		v_file_data.swap(v_file_data);
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
void SplitString(vector<char>& vec,string str) {
	int len = str.length();
	const char* p = str.data();
	for (int i = 0; i < len; i++) {
		vec.push_back(p[i]);
	}
}
void SplitInt(vector<char>& vec, int num) {
	char str[3];
	if (num < 0) {
		str[0] = '0';
		num = -num;
	}
	else {
		str[0] = '1';
	}
	str[1] = IntToHex(num / 16);
	str[2] = IntToHex(num % 16);
	for (int i = 0; i < 3; i++) {
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
	case 10 :return'A'; break;
	case 11 :return'B'; break;
	case 12 :return'C'; break;
	case 13 :return'D'; break;
	case 14 :return'E'; break;
	case 15 :return'F'; break;
	case 0:return'0'; break;
	}
}
char HexToInt(char c1,char c2,char c3) {
	int num = 1;
	switch (c2) {
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
	switch (c3) {
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
	switch (c1) {
	case '0':num = -num ; break;
	case '1': break;
	}
	return num;
}
void CharTOSQLWCHAR(char* c,SQLWCHAR *s) {
	for (int i = 0; i < strlen(c); i++)
	{
		s[i] = c[i];
	}
	s[strlen(c)] = '\0';
}
