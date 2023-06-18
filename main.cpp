#include "mainwindow.h"
#include "utils.h"
#include <QApplication>
#include <iostream>
#include <string>
#include <windows.h>
#include "sqlite3.h"
#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

// define global SQLite3 database connection
sqlite3* db;

// WorkerThread 类用于在单独的线程中运行循环逻辑
class WorkerThread : public QThread
{
public:
    void run() override
    {
        int checkInterval = 1;

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

            // reminder check
            std::vector<Reminder> reminders = get_all_reminder(db);
            for (Reminder reminder:reminders){
                if (reminder.reached()){
                    QMessageBox::information(nullptr, "Reminder", QString::fromStdString(reminder.reminderMessage));
                }
            }
            Sleep(1000 * checkInterval);
        }
    }
};

int main(int argc, char *argv[])
{
    // open SQLite3 database connection and create tables if necessary
    int rc = sqlite3_open("data.db", &db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    if (run_sql(db, createHourLog, "creating HourLog") != SQLITE_OK)
        return 1;
    if (run_sql(db, createAppModels, "creating AppModels") != SQLITE_OK)
        return 1;
    if (run_sql(db, createCategoryModels, "creating CategoryModels") != SQLITE_OK)
        return 1;
    if (run_sql(db, createReminderModels, "creating ReminderModels") != SQLITE_OK)
        return 1;

    QApplication a(argc, argv);
    MainWindow w(db);
    w.show(); 


    // 托盘图标右键菜单,实现退出
    QSystemTrayIcon trayIcon(QIcon::fromTheme("applications-engineering"));
    trayIcon.show();
    QMenu menu;
    QAction *exitAction = menu.addAction("退出");
    QObject::connect(exitAction, &QAction::triggered, &app, &QApplication::quit);
    trayIcon.setContextMenu(&menu);


    // 创建 WorkerThread 实例并启动线程
    WorkerThread workerThread;
    workerThread.start();

    // 进入 Qt 事件循环
    int result = a.exec();

    // 等待 workerThread 完成并退出
    workerThread.quit();
    workerThread.wait();

    // close SQLite3 database connection
    sqlite3_close(db);

    return result;
}
