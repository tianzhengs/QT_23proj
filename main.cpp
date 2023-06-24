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

sqlite3* db;

std::vector<std::string> reminder_executed;
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
                //std::cout << current_process << std::endl;
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
                //std::cerr<<reminder.reminderTimeLimit()<<std::endl;
                if (reminder.reached()) {
                    if (std::find(reminder_executed.begin(), reminder_executed.end(), reminder.reminderName) == reminder_executed.end()){
                        QMessageBox::information(nullptr, "Reminder", QString::fromStdString(reminder.reminderMessage));
                        reminder_executed.push_back(reminder.reminderName);
                    }

                }
            }
            Sleep(1000 * checkInterval);
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
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

    int result = a.exec();

    // 等待 workerThread 完成并退出
    workerThread.quit();
    workerThread.wait();

    sqlite3_close(db);

    return result;
}
