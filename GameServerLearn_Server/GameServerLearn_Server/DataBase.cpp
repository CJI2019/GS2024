#pragma once
#include "stdafx.h"
#include "DataBase.h"

WCHAR* CharToWCHAR(const char* cStr);
WCHAR* IntToWCHAR(int num);
void PrintError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

DataBase::DataBase()
{
    m_HDBC = nullptr;
    m_HENV = nullptr;
    
    m_DBcommand = LOGIN;
    DB_Init();
    DB_Connect();
}

DataBase::~DataBase()
{
    if (m_HDBC) {
        SQLDisconnect(m_HDBC);
        SQLFreeHandle(SQL_HANDLE_DBC, m_HDBC);
    }
    if (m_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, m_HENV);
    }
}

void DataBase::DB_Init()
{
    setlocale(LC_ALL, "korean");
    std::wcout.imbue(std::locale("korean"));

    SQLRETURN retcode;

    // Allocate environment handle  
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_HENV);

    // Set the ODBC version environment attribute  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLSetEnvAttr(m_HENV, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

        // Allocate connection handle  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_HENV, &m_HDBC);

            // Set login timeout to 5 seconds  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                cout << "Connet Ready..." << endl;
            }
            else {
                cout << "Allocate connection handle - Fail" << endl;
            }
        }
    }
}

void DataBase::DB_Connect()
{
    SQLRETURN retcode;

    SQLSetConnectAttr(m_HDBC, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    char odbc_id[256];
    cout << "DB : ";
    std::cin >> odbc_id; // odbc_game_server_2019182042
    auto p = CharToWCHAR(odbc_id);
    // Connect to data source  
    retcode = SQLConnect(m_HDBC, (SQLWCHAR*)p, SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
    free(p);

    // Allocate statement handle  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        cout << "DB Connect Complete" << endl;
    }
    else {
        cout << "DB Connect Fail" << endl;
    }
}

void DataBase::DB_Process(void* pData, DB_COMMAND command)
{
    SQLRETURN retcode;
    SQLHSTMT hstmt = nullptr;
    SQLWCHAR szuser_id[NAME_SIZE*2]; //wchar_t 변수 원소당 2바이트임에 주의
    SQLINTEGER  szxpos = 0,szypos = 0 , szhp = 0, szmaxhp = 0, szlevel = 0, szexp = 0;
    SQLLEN cbuser_id = 0, cbxpos = 0, cbypos = 0, cbhp = 0, cbmaxhp = 0, cblevel = 0, cbexp = 0;

    int x = 0;
   // std::cout << "DB command process START!\n";

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_HDBC, &hstmt);
    switch (command)
    {
    case LOGIN: {
        DB_ID_INFO* db_info = static_cast<DB_ID_INFO*>(pData);
        constexpr int SQL_SELECT_ID_BUFFER_SIZE = 64 + NAME_SIZE * 2;

        SQLWCHAR sql_select_id[SQL_SELECT_ID_BUFFER_SIZE];
        WCHAR* w_user_id = CharToWCHAR(db_info->m_strUserid.c_str());
        swprintf(sql_select_id, SQL_SELECT_ID_BUFFER_SIZE, L"EXEC select_id %s", w_user_id);
        m_mtxHstmt.lock();
        retcode = SQLExecDirect(hstmt, (SQLWCHAR*)sql_select_id, SQL_NTS);  // id 20인 객체 찾기
        
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            //szuser_id 는 문자열타입이므로 bufferlength 인자를 정확하게 넣어주어야 오류 발생 안함.
            retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &szuser_id,NAME_SIZE*2 , &cbuser_id); // 문자열 타입이 아닌 경우 BufferLength는 무시됨.
            retcode = SQLBindCol(hstmt, 2, SQL_INTEGER, &szxpos, 1, &cbxpos);
            retcode = SQLBindCol(hstmt, 3, SQL_INTEGER, &szypos, 1, &cbypos);
            retcode = SQLBindCol(hstmt, 4, SQL_INTEGER, &szhp, 1, &cbhp);
            retcode = SQLBindCol(hstmt, 5, SQL_INTEGER, &szmaxhp, 1, &cbmaxhp);
            retcode = SQLBindCol(hstmt, 6, SQL_INTEGER, &szlevel, 1, &cblevel);
            retcode = SQLBindCol(hstmt, 7, SQL_INTEGER, &szexp, 1, &cbexp);

            // Fetch and print each row of data. On an error, display a message and exit.  
            for (int i = 0; ; i++) {
                retcode = SQLFetch(hstmt);
                if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
                    PrintError(hstmt, SQL_HANDLE_STMT, retcode);
                }
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
                {
                    //printf("%4d: %S %4d %4d\n", i + 1, szuser_id, szxpos, szypos);
                }
                else {
                    // sql 데이터를 모두 받음.
                    //cout << "SQLfetch Error" << endl;
                    break;
                }
            }

            size_t user_id_Len = db_info->m_strUserid.size();
            szuser_id[user_id_Len] = '\0';
            if (!wcscmp(szuser_id, w_user_id)) {
                db_info->m_xPos = szxpos;
                db_info->m_yPos = szypos;
                db_info->m_hp = szhp;
                db_info->m_maxhp = szmaxhp;
                db_info->m_level = szlevel;
                db_info->m_exp = szexp;
            }
            else {
                db_info->m_xPos = -1;
                db_info->m_yPos = -1;
            }
        }
        m_mtxHstmt.unlock();
        free(w_user_id);
        break;
    }
    case REGISTER: {
        DB_ID_INFO* db_info = static_cast<DB_ID_INFO*>(pData);
        constexpr int SQL_REGISTER_ID_BUFFER_SIZE = 128 + NAME_SIZE * 2;

        SQLWCHAR sql_register_id[SQL_REGISTER_ID_BUFFER_SIZE];// = L"EXEC register_id ";//띄어쓰기 있음.
        WCHAR* w_user_id = CharToWCHAR(db_info->m_strUserid.c_str());
        swprintf(sql_register_id, SQL_REGISTER_ID_BUFFER_SIZE,
            L"EXEC register_id %s, %d, %d, %d, %d, %d, %d",
            w_user_id, db_info->m_xPos, db_info->m_yPos, db_info->m_hp, db_info->m_maxhp, db_info->m_level, db_info->m_exp);
        m_mtxHstmt.lock();
        retcode = SQLExecDirect(hstmt, (SQLWCHAR*)sql_register_id, SQL_NTS); // 테이블 상태 추가
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            PrintError(hstmt, SQL_HANDLE_STMT, retcode);
        }
        m_mtxHstmt.unlock();
        free(w_user_id);
        break;
    }
    case EXIT: {
        DB_ID_INFO* db_info = static_cast<DB_ID_INFO*>(pData);
        constexpr int SQL_EXIT_ID_BUFFER_SIZE = 64 + NAME_SIZE * 2;

        SQLWCHAR sql_user_data_update[SQL_EXIT_ID_BUFFER_SIZE];// = L"EXEC register_id %s, %d, %d";
        WCHAR* w_user_id = CharToWCHAR(db_info->m_strUserid.c_str());
        swprintf(sql_user_data_update, SQL_EXIT_ID_BUFFER_SIZE,
            L"EXEC exit_id %s, %d, %d, %d, %d, %d, %d",
            w_user_id, db_info->m_xPos, db_info->m_yPos, db_info->m_hp, db_info->m_maxhp, db_info->m_level, db_info->m_exp);

        m_mtxHstmt.lock();
        retcode = SQLExecDirect(hstmt, (SQLWCHAR*)sql_user_data_update, SQL_NTS); // 테이블 상태 업데이트
        
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            PrintError(hstmt, SQL_HANDLE_STMT, retcode);
        }
        else {
            //printf("[DB] (%d,%d) 좌표 저장 완료", g_db_xPos, g_db_yPos);
        }
        m_mtxHstmt.unlock();
        free(w_user_id);
        break;
    }
    default:
        break;
    }
    // Process data  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        SQLCancel(hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }

}

void PrintError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
    SQLSMALLINT iRec = 0;
    SQLINTEGER iError;
    WCHAR wszMessage[1000];
    WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
    if (RetCode == SQL_INVALID_HANDLE) {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }
    while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
        (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
        // Hide data truncated..
        if (wcsncmp(wszState, L"01004", 5)) {
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
        }
    }
}

WCHAR* CharToWCHAR(const char* cStr)
{
    // 변환된 와이드 문자열을 저장할 버퍼
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, cStr, -1, NULL, 0);
    WCHAR* wstr = (WCHAR*)malloc(requiredSize * sizeof(WCHAR));

    // 다중 바이트 문자열을 와이드 문자열로 변환
    MultiByteToWideChar(CP_ACP, 0, cStr, -1, wstr, requiredSize);

    return wstr;
}

WCHAR* IntToWCHAR(int num)
{
    // 변환된 와이드 문자열을 저장할 버퍼
    WCHAR* buffer = new WCHAR[20];

    // 정수형 값을 WCHAR 문자열로 변환
    swprintf(buffer, 20, L"%d", num);

    return buffer;
}