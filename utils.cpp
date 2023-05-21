#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

#include "sqlite3.h"


// 表结构
std::string createHourLog = "CREATE TABLE IF NOT EXISTS HourLog (date INTEGER, app INTERER, time INTEGER)";
std::string createAppModels = "CREATE TABLE IF NOT EXISTS AppModels (app INTEGER PRIMARY KEY AUTOINCREMENT, file VARCHAR(255) UNIQUE, name nvarchar NULL DEFAULT '', CategoryID int NULL DEFAULT 0)";
std::string createCategoryModels = "CREATE TABLE IF NOT EXISTS CategoryModels (CategoryID INTEGER PRIMARY KEY,name nvarchar NULL DEFAULT '')";




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
    return SQLITE_OK;
}

int get_appid_from_path(sqlite3* db, std::string processName)
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

    // get file name from file
    size_t lastSeparator = processName.find_last_of("/\\");
    
    // Find the position of the last dot (file extension)
    size_t lastDot = processName.find_last_of(".");
    
    // Extract the substring between the last separator and the last dot
    std::string fileName = processName.substr(lastSeparator + 1, lastDot - lastSeparator - 1);
    
    // if processName doesn't exist yet, insert into AppModels table
    if (result == 0)
    {
        query = "INSERT INTO AppModels (file,name) VALUES ('" + processName + "','" + fileName + "')";
        if (run_sql(db, query, "")!= SQLITE_OK){return SQLITE_ERROR;}
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
    int appid=get_appid_from_path(db, app);

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

// ------------------------------- 分组 Tab

// Return a vector<int> containing all CategoryIDs from the CategoryModels table
std::vector<int> get_all_category(sqlite3* db)
{
    std::vector<int> category_ids;
    std::string query = "SELECT CategoryID FROM CategoryModels";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        // Execute statement and loop through rows
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int category_id = sqlite3_column_int(stmt, 0);
            category_ids.push_back(category_id);
        }
        // Finalize statement
        sqlite3_finalize(stmt);
        return category_ids;
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
        return std::vector<int>(); // Return an empty vector on error
    }
    
}

// Rename a category in the CategoryModels table
bool rename_category(sqlite3* db, int CategoryID, const std::string& new_name)
{
    // Update the name for the given CategoryID in the CategoryModels table
    std::string query = "UPDATE CategoryModels SET name = '" + new_name + "' WHERE CategoryID = " + std::to_string(CategoryID);
    if (run_sql(db, query, "UPDATE CategoryModels")!= SQLITE_OK)
    {
        return false;
    }

    return true;
}

// Add a new category to the CategoryModels table
bool add_category(sqlite3* db, const std::string& category_name)
{
    // Insert the new category into the CategoryModels table
    std::string query = "INSERT INTO CategoryModels (name) VALUES ('" + category_name + "')";
    if (run_sql(db, query, "INSERT INTO CategoryModels") != SQLITE_OK)
    {
        return false;
    }

    return true;
}

// Delete a category from the CategoryModels table
bool delete_category(sqlite3* db, int CategoryID)
{
    // Update the CategoryID for all apps in the AppModels table that belong to the category being deleted
    std::string query = "UPDATE AppModels SET CategoryID = 0 WHERE CategoryID = " + std::to_string(CategoryID);
    if (run_sql(db, query, "UPDATE AppModels")!= SQLITE_OK)
    {
        return false;
    }

    // Delete the category from the CategoryModels table
    query = "DELETE FROM CategoryModels WHERE CategoryID = " + std::to_string(CategoryID);
    if (run_sql(db, query, "DELETE FROM CategoryModels")!= SQLITE_OK)
    {
        return false;
    }

    return true;
}

// Get a vector of app names under a specific category
std::vector<std::string> get_apps_under_category(sqlite3* db, int CategoryID)
{
    std::vector<std::string> app_names;
    std::string query = "SELECT name FROM AppModels WHERE CategoryID = " + std::to_string(CategoryID);
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        // Execute statement and loop through rows
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char* app_name = sqlite3_column_text(stmt, 0);
            app_names.push_back(std::string(reinterpret_cast<const char*>(app_name)));
        }
        // Finalize statement
        sqlite3_finalize(stmt);
        return app_names;
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
        return std::vector<std::string>(); // Return an empty vector on error
    }
}

// Move an app to a specific category
bool move_app_to_category(sqlite3* db, const std::string& app_name, int newCategoryId)
{
    // Update the CategoryID for the app in the AppModels table
    std::string query = "UPDATE AppModels SET CategoryID = " + std::to_string(newCategoryId) + " WHERE name = " + app_name;
    if (run_sql(db, query, "UPDATE AppModels") != SQLITE_OK)
    {
        return false;
    }

    return true;
}

