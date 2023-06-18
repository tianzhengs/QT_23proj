#ifndef MYADDREMINDER_H
#define MYADDREMINDER_H

#include <QWidget>
#include "sqlite3.h"


namespace Ui {
class MyAddReminder;
}

class MyAddReminder : public QWidget
{
    Q_OBJECT

public:
    explicit MyAddReminder(sqlite3* connection,QWidget *parent = nullptr);
    ~MyAddReminder();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MyAddReminder *ui;
    sqlite3* db; // 添加数据库连接成员变量

};
#endif // MYADDREMINDER_H
