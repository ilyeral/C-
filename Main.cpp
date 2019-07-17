#include "FileSQL.h"
#include<iostream>
using namespace std;
int main() {
	FileSQL fileSQL;
	fileSQL.Open();
	while (true) {
		cout<<"输入文件路径"<<endl;
		char filePath[256];
		cin >> filePath;
		//fileSQL.Insert(filePath);
		fileSQL.Select();
	}
	fileSQL.Close();
	return 0;
}
