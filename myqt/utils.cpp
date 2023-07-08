#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

#include "sqlite3.h"
#include "utils.h"

// 表结构
std::string createHourLog = "CREATE TABLE IF NOT EXISTS HourLog (date INTEGER, app INTERER, time INTEGER)";
std::string createAppModels = "CREATE TABLE IF NOT EXISTS AppModels (app INTEGER PRIMARY KEY AUTOINCREMENT, file VARCHAR(255) UNIQUE, name nvarchar NULL DEFAULT '', CategoryID int NULL DEFAULT 0)";
std::string createCategoryModels = "CREATE TABLE IF NOT EXISTS CategoryModels (CategoryID INTEGER PRIMARY KEY AUTOINCREMENT,name nvarchar NULL DEFAULT '')";
std::string createReminderModels = "CREATE TABLE IF NOT EXISTS ReminderModels (ReminderID INTEGER PRIMARY KEY AUTOINCREMENT,ReminderName NVARCHAR,ReminderBindCategory INT,ReminderTimeLimit INT,ReminderMessage NVARCHAR)";

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

int get_appid_from_path(sqlite3* db, std::string fpath)
{
    std::string query = "SELECT COUNT(*) FROM AppModels WHERE file='" + fpath + "'";
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
    size_t lastSeparator = fpath.find_last_of("/\\");

    // Find the position of the last dot (file extension)
    size_t lastDot = fpath.find_last_of(".");

    // Extract the substring between the last separator and the last dot
    std::string fileName = fpath.substr(lastSeparator + 1, lastDot - lastSeparator - 1);

    // if fpath doesn't exist yet, insert into AppModels table
    if (result == 0)
    {
        query = "INSERT INTO AppModels (file,name) VALUES ('" + fpath + "','" + fileName + "')";
        if (run_sql(db, query, "")!= SQLITE_OK){return SQLITE_ERROR;}
    }

    // prepare SQLite3 statement to select app column from AppModels table
    query = "SELECT app FROM AppModels WHERE file='" + fpath + "'";
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

// time in seconds
int insert_focused_app_to_HourLog(sqlite3* db, std::string fpath, int time)
{
    // properly created if non-existent
    int appid=get_appid_from_path(db, fpath);

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

std::vector<std::pair<int, std::string>> get_all_category(sqlite3* db)
{
    std::vector<std::pair<int, std::string>> categories;
    std::string query = "SELECT CategoryID, name FROM CategoryModels";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        // Execute statement and loop through rows
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int categoryID = sqlite3_column_int(stmt, 0);
            const unsigned char* categoryName = sqlite3_column_text(stmt, 1);
            std::string categoryNameStr(reinterpret_cast<const char*>(categoryName));
            //std::cerr<<"id: "<<categoryID<<std::endl;
            categories.emplace_back(categoryID, categoryNameStr);
        }
        // Finalize statement
        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
        return std::vector<std::pair<int, std::string>>(); // Return an empty vector on error
    }

    // If no categories found, insert default category
    if (categories.empty())
    {
        int defaultCategoryID = 0;
        std::string defaultCategoryName = "未分类";
        categories.emplace_back(defaultCategoryID, defaultCategoryName);

        std::string insertQuery = "INSERT INTO CategoryModels (CategoryID, name) VALUES (" + std::to_string(defaultCategoryID) + ", '" + defaultCategoryName + "')";
        if (run_sql(db, insertQuery, "INSERT INTO CategoryModels") != SQLITE_OK)
        {
            std::cerr << "Error inserting default category: " << sqlite3_errmsg(db) << std::endl;
        }
    }


    return categories;
}

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

bool move_app_to_category(sqlite3* db, const std::string& app_name, int newCategoryId)
{
    // Update the CategoryID for the app in the AppModels table
    std::string query = "UPDATE AppModels SET CategoryID = " + std::to_string(newCategoryId) + " WHERE name = '" + app_name + "'";
    if (run_sql(db, query, "UPDATE AppModels") != SQLITE_OK)
    {
        return false;
    }

    return true;
}


// ------------------------------- 提醒 Tab

// struct Reminder {
//     int reminderID;
//     std::string reminderName;
//     int reminderBindCategory;
//     int reminderTimeLimit;
//     std::string reminderMessage;
//     sqlite3 *db;
//     bool reached();
// };

std::vector<Reminder> get_all_reminder(sqlite3* db) {
    std::vector<Reminder> reminders;

    std::string sql = "SELECT * FROM ReminderModels";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing SQLite statement: " << sqlite3_errmsg(db) << std::endl;
        return reminders;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Reminder reminder;
        reminder.reminderID = sqlite3_column_int(stmt, 0);
        reminder.reminderName = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        reminder.reminderBindCategory = sqlite3_column_int(stmt, 2);
        reminder.reminderTimeLimit = sqlite3_column_int(stmt, 3);
        reminder.reminderMessage = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        reminder.db = db;
        reminders.push_back(reminder);
    }

    sqlite3_finalize(stmt);

    return reminders;
}

bool add_reminder(sqlite3* db, const std::string& name, int CategoryID, int timelimit, const std::string& message) {
    std::string query = "INSERT INTO ReminderModels (ReminderName, ReminderBindCategory, ReminderTimeLimit, ReminderMessage) "
                        "VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, CategoryID);
        sqlite3_bind_int(stmt, 3, timelimit);
        sqlite3_bind_text(stmt, 4, message.c_str(), -1, SQLITE_STATIC);

        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return result == SQLITE_DONE;
    }

    return false;
}

bool remove_reminder(sqlite3* db, const std::string& name) {
    std::string query = "DELETE FROM ReminderModels WHERE ReminderName = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return result == SQLITE_DONE;
    }

    return false;
}

bool modify_reminder(sqlite3* db, const std::string& name, int CategoryID, int timelimit, const std::string& message) {
    std::string query = "UPDATE ReminderModels SET ReminderBindCategory = ?, ReminderTimeLimit = ?, ReminderMessage = ? "
                        "WHERE ReminderName = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, CategoryID);
        sqlite3_bind_int(stmt, 2, timelimit);
        sqlite3_bind_text(stmt, 3, message.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_STATIC);

        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return result == SQLITE_DONE;
    }

    return false;
}

bool Reminder::reached(){
    SYSTEMTIME st;
    GetLocalTime(&st);
    int today_start_time = (st.wYear * 10000 + st.wMonth * 100 + st.wDay) * 100 + 0;
    int today_end_time = (st.wYear * 10000 + st.wMonth * 100 + st.wDay) * 100 + 24;

    // Calculate the sum of usage time of apps of the current bind category on the current day
    int sumUsageTime = 0;
    std::vector<std::string> apps_thiscat = get_apps_under_category(db, reminderBindCategory);
    std::vector<std::pair<std::string, int>> appTimespan = get_usage_app_timespan(db, today_start_time, today_end_time);
    //std::cerr<<"category_size: "<<apps_thiscat.size()<<std::endl;
    for (const auto& app : appTimespan)
    {
        // Assuming the app name is stored in the reminderName member variable
        //std::cerr<<"find: "<<(std::find(apps_thiscat.begin(), apps_thiscat.end(), app.first) != apps_thiscat.end())<<std::endl;
        if (std::find(apps_thiscat.begin(), apps_thiscat.end(), app.first) != apps_thiscat.end())
        {
            //std::cerr<<app.second<<std::endl;
            sumUsageTime += app.second;
        }
    }

    // Compare the sum of usage time against the limit
    return (sumUsageTime >= reminderTimeLimit);
}

// ------------------------------- 统计 Tab
std::vector<std::pair<std::string, int>> get_usage_app_timespan(sqlite3* db, int start, int end)
{
    std::vector<std::pair<std::string, int>> appUsage;

    std::string query = "SELECT AppModels.name, SUM(HourLog.time) FROM HourLog "
                        "INNER JOIN AppModels ON HourLog.app = AppModels.app "
                        "WHERE HourLog.date >= " + std::to_string(start) +
                        " AND HourLog.date <= " + std::to_string(end) +
                        " GROUP BY HourLog.app";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            //std::cerr<<"success 1"<<std::endl;
            const unsigned char* appName = sqlite3_column_text(stmt, 0);
            //std::cerr<<std::string(reinterpret_cast<const char*>(appName))<<std::endl;
            int usageMinutes = sqlite3_column_int(stmt, 1)/60;
            //std::cerr<<usageMinutes<<std::endl;
            appUsage.push_back(std::make_pair(std::string(reinterpret_cast<const char*>(appName)), usageMinutes));
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
    }
    //if(!appUsage.empty()) std::cerr<<"not empty"<<std::endl;
    //else std::cerr<<"get empty"<<std::endl;
    return appUsage;
}

std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>> get_app_usage_timeline_day(sqlite3* db)
{
    std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>> timeline;

    std::string query = "SELECT SUBSTR(CAST(HourLog.date AS TEXT), 9, 2), AppModels.name, SUM(HourLog.time) "
                        "FROM HourLog "
                        "JOIN AppModels ON HourLog.app = AppModels.app "
                        "GROUP BY SUBSTR(CAST(HourLog.date AS TEXT), 9, 2), AppModels.name "
                        "ORDER BY SUBSTR(CAST(HourLog.date AS TEXT), 9, 2)";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        int current_hour = -1;
        std::vector<std::pair<std::string, int>> appUsage;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int hour = sqlite3_column_int(stmt, 0);
            const unsigned char* app_name = sqlite3_column_text(stmt, 1);
            int usage = sqlite3_column_int(stmt, 2);

            if (hour != current_hour)
            {
                if (current_hour != -1)
                {
                    timeline.emplace_back(current_hour, appUsage);
                    appUsage.clear();
                }
                current_hour = hour;
            }

            appUsage.emplace_back(reinterpret_cast<const char*>(app_name), usage/60);
        }

        // Add the last hour's data
        if (!appUsage.empty())
        {
            timeline.emplace_back(current_hour, appUsage);
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << "Error in SQLite3 query: " << sqlite3_errmsg(db) << std::endl;
        return std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>>();
    }

    return timeline;
}
