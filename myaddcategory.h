#ifndef MYADDCATEGORY_H
#define MYADDCATEGORY_H

#include <QWidget>
#include "sqlite3.h"


namespace Ui {
class MyAddCategory;
}

class MyAddCategory : public QWidget
{
    Q_OBJECT

public:
    explicit MyAddCategory(sqlite3* connection,QWidget *parent = nullptr);
    ~MyAddCategory();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MyAddCategory *ui;
    sqlite3* db; // 添加数据库连接成员变量

};

#endif // MYADDCATEGORY_H
