#include "mydeletereminder.h"
#include "ui_mydeletereminder.h"
#include <QSqlError>
#include <QApplication>
#include <iostream>
#include <string>
#include <windows.h>
#include "sqlite3.h"
#include <QThread>
#include <QMessageBox>
#include <mainwindow.h>

MyDeleteReminder::MyDeleteReminder(sqlite3* connection,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyDeleteReminder),db(connection)
{
    ui->setupUi(this);
}

MyDeleteReminder::~MyDeleteReminder()
{
    delete ui;
}

void MyDeleteReminder::on_pushButton_clicked()
{
    remove_reminder(db,delete_name);
    close();
}


void MyDeleteReminder::on_pushButton_2_clicked()
{
    close();
}

