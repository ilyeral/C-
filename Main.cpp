#include "FileSQL.h"
#include<iostream>
using namespace std;
int main() {
	FileSQL fileSQL;
	fileSQL.Open();
	fileSQL.TableExistence();//如数据库不存在则创建
	fileSQL.ScriptTableExistence();//如数据库不存在则创建
	while (true) {
		char cName[20];
		char cIp[20];
		char cUser[20];
		char cPassword[20];
		char cDatabase[20];
		char filePath[256];
		char fileId[10];

		cout << "请选择操作：" << endl;
		cout << "1-存文件" << endl;
		cout << "2-取文件" << endl;
		cout << "3-更新文件" << endl;
		cout << "4-删除文件" << endl;
		cout << "5-创建远程连接" << endl;
		cout << "6-删除远程连接" << endl;
		cout << "7-创建同步触发器" << endl;
		cout << "8-删除同步触发器" << endl;
		cout << "9-同步" << endl;
		cout << "操作代码:";

		int a;
		cin >> a;
		switch (a) {
		case 1:
			cout << "输入文件路径:" << endl;
			cin >> filePath;
			fileSQL.Insert(filePath);
			break;
		case 2:
			cout << "以下为文件列表，请输入文件id选择文件" << endl;
			fileSQL.SelectAll();
			cout << endl;
			cout << "文件id:";
			cin >> fileId;
			fileSQL.SelectById(fileId);
			break;
		case 3:
			cout << "输入文件路径:" << endl;
			cin >> filePath;
			fileSQL.Update(filePath);
			break;
		case 4:
			cout << "以下为文件列表，请输入文件id选择文件" << endl;
			fileSQL.SelectAll();
			cout << endl;
			cout << "文件id:";
			cin >> fileId;
			fileSQL.Delete(fileId);
			break;
		case 5:

			cout << "输入ip地址：";
			cin >> cIp;
			cout << "输入用户名：";
			cin >> cUser;
			cout << "输入密码：";
			cin >> cPassword;
			fileSQL.BindDataBase(cIp,cUser,cPassword);

			break;
		case 6:
			cout << "当前所有连接："<<endl;
			fileSQL.SelectAllBind();
			cout << "请输入要删除的连接名：";
			cin >> cName;
			fileSQL.UnBindDataBase(cName);
			break;
		case 7:
			cout << "当前所有连接：" << endl;
			fileSQL.SelectAllBind();
			cout << "请输入要同步的服务器连接名：";
			cin >> cName;
			cout << "请输入要同步的服务器数据库名：";
			cin >> cDatabase;
			fileSQL.CreateTriggerForBind(cName,cDatabase);
			break;
		case 8:
			cout << "当前所有连接：" << endl;
			fileSQL.SelectAllBind();
			cout << "请输入要删除的连接名：";
			cin >> cName;
			fileSQL.DropTriggerForBind(cName);
			break;
		case 9:
			fileSQL.DoScript();
			break;
		default:
			cout << "命令无效，请重新输入！" << endl;
			break;
		}
	}
	
	fileSQL.Close();
	return 0;
}
