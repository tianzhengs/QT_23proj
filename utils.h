#include <string>
#include <vector>

#include "sqlite3.h"

// 表结构
extern std::string createHourLog;
extern std::string createAppModels;
extern std::string createCategoryModels;

extern std::string get_current_process();

extern int run_sql(sqlite3* db, std::string sql, std::string marker);

extern int get_appid_from_path(sqlite3* db, std::string fpath);

extern int insert_focused_app_to_HourLog(sqlite3* db, std::string fpath, int time);

// ------------------------------- 分组 Tab

extern std::vector<std::pair<int, std::string>> get_all_category(sqlite3* db);

extern bool rename_category(sqlite3* db, int CategoryID, const std::string& new_name);

extern bool add_category(sqlite3* db, const std::string& category_name);

extern bool delete_category(sqlite3* db, int CategoryID);

extern std::vector<std::string> get_apps_under_category(sqlite3* db, int CategoryID);

extern bool add_app_to_catogory(sqlite3* db, std::string app, int CategoryID);

extern bool move_app_to_category(sqlite3 *db, const std::string &app_name, int newCategoryId);