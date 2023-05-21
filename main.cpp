#include <iostream>
#include <string>
#include <windows.h>

#include "sqlite3.h"
#include "utils.h"






// define global SQLite3 database connection
sqlite3* db;
int main()
{
    int checkInterval = 1;
    int x;
    // open SQLite3 database connection and create tables if necessary
    int rc = sqlite3_open("data.db", &db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl;
        std::cin >> x;
    }

    if (run_sql(db, createHourLog, "creating HourLog")!= SQLITE_OK){std::cin >> x;}
    if (run_sql(db, createAppModels, "creating AppModels")!= SQLITE_OK){std::cin >> x;}
    if (run_sql(db, createCategoryModels, "creating CategoryModels")!= SQLITE_OK){std::cin >> x;}

    // main loop to retrieve focused app info and insert into database
    while (1)
    {
        std::string current_process = get_current_process();
        if (!current_process.empty())
        {
            std::cout << current_process << std::endl;
            // insert focused app data into HourLog table
            insert_focused_app_to_HourLog(db, current_process, checkInterval);
        }
        else
        {
            std::cout << "Error!!" << std::endl;
        }
        Sleep(1000*checkInterval);
    }

    // close SQLite3 database connection
    sqlite3_close(db);
    return 0;
}