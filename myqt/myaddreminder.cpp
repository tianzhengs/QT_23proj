#include "myaddreminder.h"
#include "ui_myaddreminder.h"
#include <QSqlError>
#include <QApplication>
#include <iostream>
#include <string>
#include <windows.h>
#include "sqlite3.h"
#include <QThread>
#include <QMessageBox>
#include <mainwindow.h>

MyAddReminder::MyAddReminder(sqlite3* connection,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyAddReminder),db(connection)
{
    ui->setupUi(this);
}

MyAddReminder::~MyAddReminder()
{
    delete ui;
}

void MyAddReminder::on_pushButton_clicked()
{
    emit messageReceived();
    close();
}

void MyAddReminder::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 在窗口展示时执行的函数
    ui->lineEdit->clear();
    ui->lineEdit_3->clear();
    ui->lineEdit_4->clear();
    QComboBox* choose=ui->comboBox;
    choose->clear();
    //std::cerr<<"print category"<<std::endl;
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);
    for(const auto& category : categories){
        //std::cerr<<category.second<<std::endl;
        choose->addItem(QString::fromStdString(category.second));
    }
}

void MyAddReminder::on_pushButton_2_clicked()
{
    QString reminderName = ui->lineEdit->text();
    QString reminderBindCategory=ui->comboBox->currentText();
    QString reminderTimeLimit=ui->lineEdit_3->text();
    QString reminderMessage=ui->lineEdit_4->text();
    int Categoryid;
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);
    for(const auto& category : categories){
        if(category.second==reminderBindCategory.toStdString()) Categoryid=category.first;
    }
    bool success = add_reminder(db, reminderName.toStdString(),Categoryid,reminderTimeLimit.toInt(),reminderMessage.toStdString());
    if(!success)
        std::cerr << "Error opening SQLite3 database: ";
    else
    {
        QMessageBox::information(this, "Success", "Reminder added successfully.");
        ui->lineEdit->clear();
    }
    emit messageReceived();
    close();
}
