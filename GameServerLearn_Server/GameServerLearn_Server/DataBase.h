#pragma once
#include <locale>
#include <sqlext.h>  

#define DB DataBase::GetInstance()

enum DB_COMMAND { LOGIN = 0,REGISTER, EXIT };

struct DB_ID_INFO {
	std::string m_strUserid;
	int m_xPos;
	int m_yPos;
	int m_hp;
	int m_maxhp;
	int m_level;
	int m_exp;
};

class DataBase
{
private:
	DataBase();
public:
	static DataBase& GetInstance(){
		static DataBase database;
		return database;
	}

	~DataBase();
protected:
	SQLHDBC m_HDBC;
	SQLHENV m_HENV;

	DB_COMMAND m_DBcommand;

	mutex m_mtxHstmt;
public:
	void DB_Init();
	void DB_Connect();
	void DB_Process(void* pData, DB_COMMAND command);

};
