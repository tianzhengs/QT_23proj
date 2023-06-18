#ifndef _utils_H_
#define _utils_H_
#include <string>
#include <vector>

#include "sqlite3.h"

// 表结构
extern std::string createHourLog;
extern std::string createAppModels;
extern std::string createCategoryModels;
extern std::string createReminderModels;

extern std::string get_current_process();

extern int run_sql(sqlite3* db, std::string sql, std::string marker);

extern int get_appid_from_path(sqlite3* db, std::string fpath);

extern int insert_focused_app_to_HourLog(sqlite3* db, std::string fpath, int time);



// ------------------------------- 分组 Tab

/**
 * Retrieves all categories along with their IDs from the database.
 *
 * @param db The SQLite3 database connection.
 * @return A vector of pairs, where each pair contains the category ID and name.
 */
extern std::vector<std::pair<int, std::string>> get_all_category(sqlite3* db);

/**
 * Renames a category with the given CategoryID.
 *
 * @param db The SQLite3 database connection.
 * @param CategoryID The ID of the category to rename.
 * @param new_name The new name for the category.
 * @return True if the category was renamed successfully, false otherwise.
 */
extern bool rename_category(sqlite3* db, int CategoryID, const std::string& new_name);

/**
 * Adds a new category with the specified name to the database.
 *
 * @param db The SQLite3 database connection.
 * @param category_name The name of the new category.
 * @return True if the category was added successfully, false otherwise.
 */
extern bool add_category(sqlite3* db, const std::string& category_name);

/**
 * Deletes a category with the given CategoryID from the database.
 *
 * @param db The SQLite3 database connection.
 * @param CategoryID The ID of the category to delete.
 * @return True if the category was deleted successfully, false otherwise.
 */
extern bool delete_category(sqlite3* db, int CategoryID);

/**
 * Retrieves all apps under the specified category from the database.
 *
 * @param db The SQLite3 database connection.
 * @param CategoryID The ID of the category.
 * @return A vector of app names under the specified category.
 */
extern std::vector<std::string> get_apps_under_category(sqlite3* db, int CategoryID);

/**
 * Moves an app to a new category.
 *
 * @param db The SQLite3 database connection.
 * @param app_name The name of the app to move.
 * @param newCategoryId The ID of the new category.
 * @return True if the app was moved successfully, false otherwise.
 */
extern bool move_app_to_category(sqlite3* db, const std::string& app_name, int newCategoryId);

// ------------------------------- 提醒 Tab
struct Reminder {
    int reminderID;
    std::string reminderName;
    int reminderBindCategory;
    int reminderTimeLimit;
    std::string reminderMessage;
    sqlite3 *db;
    bool reached();
};

/**
 * Retrieves all the reminder objects from the database.
 *
 * @param db The SQLite3 database connection.
 * @return A vector of Reminder objects.
 */
std::vector<Reminder> get_all_reminder(sqlite3* db) ;

/**
 * Adds a new reminder to the database.
 *
 * @param db The SQLite3 database connection.
 * @param name The name of the reminder.
 * @param CategoryID The ID of the category to bind the reminder.
 * @param timelimit The time limit in minutes for the reminder.
 * @param message The reminder message.
 * @return True if the reminder was added successfully, false otherwise.
 */
bool add_reminder(sqlite3* db, const std::string& name, int CategoryID, int timelimit, const std::string& message);

/**
 * Removes a reminder from the database.
 *
 * @param db The SQLite3 database connection.
 * @param name The name of the reminder to remove.
 * @return True if the reminder was removed successfully, false otherwise.
 */
bool remove_reminder(sqlite3* db, const std::string& name);

/**
 * Modifies an existing reminder in the database.
 *
 * @param db The SQLite3 database connection.
 * @param name The name of the reminder to modify.
 * @param CategoryID The ID of the category to bind the reminder.
 * @param timelimit The new time limit in minutes for the reminder.
 * @param message The new reminder message.
 * @return True if the reminder was modified successfully, false otherwise.
 */
bool modify_reminder(sqlite3* db, const std::string& name, int CategoryID, int timelimit, const std::string& message);

// ------------------------------- 统计 Tab

/**
 * Retrieves the usage timespan of apps within the specified start and end dates.
 *
 * @param db The SQLite3 database connection.
 * @param start The start date in the format YYYYMMDDHH.
 * @param end The end date in the format YYYYMMDDHH.
 * @return A vector of pairs, where each pair contains the app name and its used minutes within the timespan.
 */
std::vector<std::pair<std::string, int>> get_usage_app_timespan(sqlite3* db, int start, int end);

/**
 * Retrieves the app usage timeline for each hour of the day.
 *
 * @param db The SQLite3 database connection.
 * @return A vector of pairs, where each pair contains the hour (0-23) and a vector of pairs representing the app name and its usage in minutes during that hour.
 */
std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>> get_app_usage_timeline_day(sqlite3* db);
#endif
