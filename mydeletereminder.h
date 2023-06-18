#ifndef MYDELETEREMINDER_H
#define MYDELETEREMINDER_H

#include <QDialog>
#include <QWidget>
#include "sqlite3.h"


namespace Ui {
class MyDeleteReminder;
}

class MyDeleteReminder : public QWidget
{
    Q_OBJECT

public:
    std::string delete_name;
    explicit MyDeleteReminder(sqlite3* connection,QWidget *parent = nullptr);
    ~MyDeleteReminder();

private slots:

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MyDeleteReminder *ui;
    sqlite3* db; // 添加数据库连接成员变量


};
#endif // MYDELETEREMINDER_H
