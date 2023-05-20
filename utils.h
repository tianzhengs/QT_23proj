#ifndef UTILS
#define UTILS

std::string get_current_process();

int run_sql(sqlite3 *db, std::string sql, std::string marker);

int safe_get_appid(sqlite3 *db, std::string processName);

int insert_focused_app_to_HourLog(sqlite3 *db, std::string app, int time);

#endif