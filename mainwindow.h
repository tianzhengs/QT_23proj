#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "qmimedata.h"
#include <QTreeWidget>
#include <QCloseEvent>
#include <QMainWindow>
#include <myaddcategory.h>
#include <utils.h>
#include <QSqlDatabase>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QSqlQuery>
#include <myaddreminder.h>
#include <mydeletereminder.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(sqlite3* connection,QWidget *parent = nullptr);
    typedef std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>> TimelineData;
    void displayReminders();
    void initTreeWidget();
    void createTimeline();
    void createtimesum();
    ~MainWindow();


private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_reminder1_clicked();

    void on_pushButton_reminder3_clicked();

    void on_pushButton_reminder2_clicked();

    void on_ReminderList_clicked(const QModelIndex &index);

    void on_CategoryList_doubleClicked(const QModelIndex &index);

    void onAppItemDoubleClicked(QTreeWidgetItem* item, int column);


    void on_dateEdit_userDateChanged(const QDate &date);

    void on_dateEdit_2_userDateChanged(const QDate &date);

public:
    void closeEvent(QCloseEvent *event) override;
    QTreeWidgetItem* selectedAppItem;

    void updateTimeline(const TimelineData& timelineData);

private:
    Ui::MainWindow *ui;
    sqlite3* db; // 添加数据库连接成员变量
    MyAddReminder newreminder;
    QTreeWidget* treeWidget;
    QGraphicsView* timelineView;
    QTreeWidgetItem* selectedItem;
    QGraphicsScene* timelineScene;
    MyDeleteReminder newdeletereminder;
    QSqlQuery sql;
};
#endif // MAINWINDOW_H
