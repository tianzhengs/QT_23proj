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
    close();
}


void MyAddReminder::on_pushButton_2_clicked()
{
    QString reminderName = ui->lineEdit->text();
    QString reminderBindCategory=ui->lineEdit_2->text();
    QString reminderTimeLimit=ui->lineEdit_3->text();
    QString reminderMessage=ui->lineEdit_4->text();
    bool success = add_reminder(db, reminderName.toStdString(),reminderBindCategory.toInt(),reminderTimeLimit.toInt(),reminderMessage.toStdString());
    if(!success)
        std::cerr << "Error opening SQLite3 database: ";
    else
    {
        QMessageBox::information(this, "Success", "Reminder added successfully.");
        ui->lineEdit->clear();
    }
    close();
}
