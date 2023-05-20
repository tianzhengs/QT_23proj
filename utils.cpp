#include <iostream>
#include <windows.h>
#include <string>

#include "sqlite3.h"

// define global SQLite3 database connection

HMODULE hModule = LoadLibrary(TEXT("psapi.dll"));

std::string get_current_process()
{
    HWND hwndFocused = GetForegroundWindow(); // get focused window handle
    if (hwndFocused)
    {
        DWORD dwProcessId;
        GetWindowThreadProcessId(hwndFocused, &dwProcessId);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
        if (hProcess)
        {
            if (hModule)
            {
                char szProcessName[MAX_PATH] = {0};
                typedef DWORD(WINAPI *PFN_GetModuleFileNameExA)(HANDLE, HMODULE, LPSTR, DWORD);
                PFN_GetModuleFileNameExA pfnGetModuleFileNameExA = (PFN_GetModuleFileNameExA)GetProcAddress(hModule, "GetModuleFileNameExA");
                if (pfnGetModuleFileNameExA)
                {
                    pfnGetModuleFileNameExA(hProcess, NULL, szProcessName, MAX_PATH);
                    std::string processName(szProcessName);
                    // FreeLibrary(hModule);
                    CloseHandle(hProcess);
                    return processName;
                }
                // FreeLibrary(hModule);
            }
            CloseHandle(hProcess);
        }
    }
    return "";
}

int run_sql(sqlite3* db, std::string sql, std::string marker)
{
    char* errMsg = NULL;
    if (sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL)!= SQLITE_OK)
    {
        std::cerr << "Error in "<< marker << ": " << sqlite3_errmsg(db) << std::endl;
        return SQLITE_ERROR;
    }
}

int safe_get_appid(sqlite3* db, std::string processName)
{
    std::string query = "SELECT COUNT(*) FROM AppModels WHERE file='" + processName + "'";
    sqlite3_stmt* stmt;
    int result=0;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        // execute statement
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result = sqlite3_column_int(stmt, 0);
        }
        // finalize statement
        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
        return SQLITE_ERROR;
    }

    // if processName doesn't exist yet, insert into AppModels table
    if (result == 0)
    {
        query = "INSERT INTO AppModels (file) VALUES ('" + processName + "')";
        if (run_sql(db, query, "safe_get_appid")!= SQLITE_OK){return SQLITE_ERROR;}
    }

    // prepare SQLite3 statement to select app column from AppModels table
    query = "SELECT app FROM AppModels WHERE file='" + processName + "'";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        // execute statement
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result = sqlite3_column_int(stmt, 0); // get app column (int) of result
        }
        else
        {
            result = 0;
        }
        // finalize statement
        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
        return SQLITE_ERROR;
    }

    return result;
}

int insert_focused_app_to_HourLog(sqlite3* db, std::string app, int time)
{
    // properly created if non-existent
    int appid=safe_get_appid(db, app);

    SYSTEMTIME st;
    GetLocalTime(&st);
    int date = (st.wYear * 10000 + st.wMonth * 100 + st.wDay) * 100 + st.wHour;

    std::string query = "SELECT time FROM HourLog WHERE app=" + std::to_string(appid) + " AND date=" + std::to_string(date);
    sqlite3_stmt* stmt;
    int already_time = 0;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        // execute statement
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            already_time = sqlite3_column_int(stmt, 0);
        }
        // finalize statement
        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
        return SQLITE_ERROR;
    }
    if (already_time==0)
    {
        query = "INSERT INTO HourLog (date, app, time) VALUES (" + std::to_string(date) + ", " + std::to_string(appid) + ", " + std::to_string(time) + ")";
        if (run_sql(db, query, "INSERT INTO HourLog")!= SQLITE_OK){return SQLITE_ERROR;}
        return SQLITE_OK;
    }
    else
    {   
        std::string new_time = std::to_string(already_time + time);
        query = "UPDATE HourLog SET time = " + new_time + " WHERE date = " + std::to_string(date) + " AND app = " + std::to_string(appid);
        if (run_sql(db, query, "UPDATE HourLog")!= SQLITE_OK){return SQLITE_ERROR;}
        return SQLITE_OK;
    }
    return SQLITE_ERROR;
    
}


