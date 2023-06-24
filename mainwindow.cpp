#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlError>
#include <QInputDialog>
#include <QMessageBox>
#include <utils.h>
#include <QDragEnterEvent>
#include <sstream>
#include <iomanip>
#include <QAction>
#include <set>
#include <iostream>
#include <vector>
#include <string>
#include <QMenu>
#include <map>
#include <random>
#include <algorithm>
#include <QMimeData>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

typedef std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>> TimelineData;
// 根据appname生成随机颜色，要求颜色不相同且容易区分
std::string generateColor(const std::string& str)
{
    std::hash<std::string> hashFunction;
    std::size_t hashValue = hashFunction(str);

    int r = (hashValue & 0xFF0000) >> 16;
    int g = (hashValue & 0x00FF00) >> 8;
    int b = hashValue & 0x0000FF;

    r = (r + 40) % 256;
    g = (g + 80) % 256;
    b = (b + 120) % 256;

    std::stringstream colorStream;
    colorStream << "#" << std::setfill('0') << std::setw(2) << std::hex << r
                << std::setw(2) << std::hex << g
                << std::setw(2) << std::hex << b;

    return colorStream.str();
}

MainWindow::MainWindow(sqlite3* connection, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db(connection),newreminder(db),newdeletereminder(db)
{
    ui->setupUi(this);
    treeWidget = ui->CategoryList;
    timelineView =ui->graphicsView;
    timelineScene = new QGraphicsScene(this);
    timelineView->setScene(timelineScene);
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabels({ "Categories" });
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);

    for (const auto& category : categories)
    {
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(treeWidget);
        categoryItem->setText(0, QString::fromStdString(category.second));
    }
    displayReminders();
    treeWidget = ui->CategoryList;
    treeWidget->setDragEnabled(true);
    treeWidget->setAcceptDrops(true);
    initTreeWidget();
    createTimeline();
    createtimesum();
}



MainWindow::~MainWindow()
{
    delete ui;
}

//添加category
void MainWindow::on_pushButton_clicked()
{
    QString categoryName = QInputDialog::getText(this, "Add Category", "Enter Category Name");

    if (!categoryName.isEmpty())
    {
        std::vector<std::pair<int, std::string>> categories = get_all_category(db);
        //category添加判重
        int CategoryID=1;
        for (const auto& category : categories) {
            if (category.second == categoryName.toStdString()) {
                 CategoryID=0;
                 break;
            }
        }
        if(CategoryID){
            add_category(db,categoryName.toStdString());
            QTreeWidgetItem* categoryItem = new QTreeWidgetItem(treeWidget);
            categoryItem->setText(0, categoryName);
        }
        else{
            QMessageBox::information(nullptr, "Error!!", "Category already exist！", QMessageBox::Ok);
        }
    }

}

//删除category
void MainWindow::on_pushButton_2_clicked()
{
    QTreeWidgetItem *selectedItem = treeWidget->currentItem();
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);
    QString categoryText = selectedItem->text(0);
    int categoryID = -1;
    for (const auto& category : categories) {
        if (category.second == categoryText.toStdString()) {
            categoryID = category.first;
            break;
        }
    }
    if (selectedItem)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Comfirmation", "Are you sure you want to delete this category?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            if (delete_category(db,categoryID))
            {
                delete selectedItem;
                 //刷新界面
                initTreeWidget();
            }
            else
            {
                QMessageBox::critical(this, "error", "deletion failed！");
            }
        }
    }
    //刷新
    initTreeWidget();
}

//category初始化
void MainWindow::initTreeWidget()
{
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabels(QStringList()<<QString::fromLocal8Bit("click twice to rename or change category"));
    treeWidget->clear();
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);

    for (const auto& category : categories)
    {
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(treeWidget);
        categoryItem->setText(0, QString::fromStdString(category.second));

        std::vector<std::string> appNames = get_apps_under_category(db, category.first);

        for (const std::string& appName : appNames)
        {
            QTreeWidgetItem* appItem = new QTreeWidgetItem(categoryItem);
            appItem->setFlags(appItem->flags() | Qt::ItemIsDragEnabled);
            appItem->setText(0, QString::fromStdString(appName));
            disconnect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onAppItemDoubleClicked(QTreeWidgetItem*, int)));
            connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onAppItemDoubleClicked(QTreeWidgetItem*, int)));
        }
    }
}

//展示reminder
void MainWindow::displayReminders() {
    QTreeWidget* treeWidget = ui->ReminderList;
    treeWidget->clear(); // 清空QTreeWidget中的所有项

    // 从数据库获取所有的reminder
    std::vector<Reminder> reminders = get_all_reminder(db);

    // 遍历reminders并将其添加到QTreeWidget中
    for (const auto& reminder : reminders) {
        QTreeWidgetItem* reminderItem = new QTreeWidgetItem(treeWidget);
        reminderItem->setText(0, QString::fromStdString(reminder.reminderName));
        reminderItem->setText(1, QString::number(reminder.reminderBindCategory));
        reminderItem->setText(2, QString::number(reminder.reminderTimeLimit));
        reminderItem->setText(3, QString::fromStdString(reminder.reminderMessage));
    }
    ui->pushButton_reminder2->hide();
}

//新增reminder
void MainWindow::on_pushButton_reminder1_clicked()
{
    newreminder.show();
    displayReminders();
}

//刷新reminder
void MainWindow::on_pushButton_reminder3_clicked()
{
    displayReminders();
}

//删除reminder
void MainWindow::on_pushButton_reminder2_clicked()
{
     newdeletereminder.show();
}

//选中reminder
void MainWindow::on_ReminderList_clicked(const QModelIndex &index0)
{
    int row;
    row=index0.row();
    auto index1=ui->ReminderList->topLevelItem(row);
    newdeletereminder.delete_name=(index1->text(0)).toStdString();
    ui->pushButton_reminder2->show();
}
//Category重命名
void MainWindow::on_CategoryList_doubleClicked(const QModelIndex &index)
{
    QTreeWidgetItem* selectedItem = treeWidget->currentItem();
    int column = index.column();
    //只能重命名cateory
    if (selectedItem && column == 0 && !selectedItem->parent())
    {
        std::vector<std::pair<int, std::string>> categories = get_all_category(db);
        QString categoryText = selectedItem->text(0);
        int categoryID = -1;
        for (const auto& category : categories) {
            if (category.second == categoryText.toStdString()) {
                categoryID = category.first;
                break;
            }
        }
        QString currentName = selectedItem->text(0);
        QString newName = QInputDialog::getText(this, "rename", "enter new name", QLineEdit::Normal, currentName);
        if (!newName.isEmpty() && newName != currentName)
        {
            //判重
            int CategoryID=1;
            for (const auto& category : categories) {
                if (category.second == newName.toStdString()) {
                    CategoryID=0;
                    break;
                }
            }
            if(!CategoryID)
            {
                QMessageBox::critical(this, "error", "rename failed");
            }
            else
            {
                std::string new_name = newName.toStdString();
                if (rename_category(db, categoryID, new_name))
                {
                    selectedItem->setText(0, newName);
                }
                else
                {
                    QMessageBox::critical(this, "error", "rename failed");
                }
            }

        }
    }
}

//时间轴建立
void MainWindow::createTimeline()
{
    timelineScene->clear();
    std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>> timelineData = get_app_usage_timeline_day(db);

    QFont labelFont("Arial", 10);
    std::map<std::string, std::string> appColors;

    int timelinePositionY = 100;
    int timeIntervalWidth = 100;
    int timeIntervalPositionX = 0;
    int Heightforblocks=5;

    for (const auto& timeInterval : timelineData)
    {
        int hour = timeInterval.first;
        const std::vector<std::pair<std::string, int>>& appUsage = timeInterval.second;

        QGraphicsTextItem* hourLabel = new QGraphicsTextItem(QString::number(hour));
        hourLabel->setPos(timeIntervalPositionX, timelinePositionY - 20);
        hourLabel->setFont(labelFont);
        timelineScene->addItem(hourLabel);

        for (const auto& app : appUsage)
        {
            std::string appName = app.first;
            int usageTime = app.second;

            int appHeight = usageTime * Heightforblocks;

            QGraphicsRectItem* appBar = new QGraphicsRectItem();
            appBar->setRect(timeIntervalPositionX, timelinePositionY, timeIntervalWidth, appHeight);

            if (appColors.find(appName) == appColors.end())
            {
                std::string color = generateColor(appName);
                appColors[appName] = color;
            }

            std::string color = appColors[appName];
            appBar->setBrush(QBrush(QColor(QString::fromStdString(color))));
            timelineScene->addItem(appBar);
        }
        timeIntervalPositionX += timeIntervalWidth;
    }
    //图例
    int legendPositionY = timelinePositionY + 150;
    int legendItemWidth = 80;
    int legendItemHeight = 20;
    int legendItemSpacing = 10;
    int legendPositionX = 0;

    std::set<std::string> uniqueAppNames;

    for (const auto& timeInterval : timelineData)
    {
        const std::vector<std::pair<std::string, int>>& appUsage = timeInterval.second;

        for (const auto& app : appUsage)
        {
            std::string appName = app.first;
            //判重，一个app只输出一个图例
            if (uniqueAppNames.find(appName) == uniqueAppNames.end())
            {
                std::string color = appColors[appName];
                //图块
                QGraphicsRectItem* legendItem = new QGraphicsRectItem();
                legendItem->setRect(legendPositionX, legendPositionY, legendItemWidth, legendItemHeight);
                legendItem->setBrush(QBrush(QColor(QString::fromStdString(color))));
                timelineScene->addItem(legendItem);

                //文字
                QGraphicsTextItem* appNameLabel = new QGraphicsTextItem(QString::fromStdString(appName));
                appNameLabel->setPos(legendPositionX + legendItemWidth + legendItemSpacing, legendPositionY);
                appNameLabel->setFont(labelFont);
                timelineScene->addItem(appNameLabel);

                legendPositionY += (legendItemHeight + legendItemSpacing);

                uniqueAppNames.insert(appName);
            }
        }
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore(); // Ignore the close event
    hide(); // Hide the window instead of closing it
};
//更改app分组
void MainWindow::onAppItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    //如果是app
    if (column == 0 && item->parent() != nullptr)
    {
        selectedItem = item;

        QString appName = item->text(column);
        QString newCategoryName = QInputDialog::getText(this, "move app", "please enter target category name for" + appName);

        if (!newCategoryName.isEmpty())
        {
            item->parent()->setText(0, newCategoryName);

            int categoryID = -1;
            std::vector<std::pair<int, std::string>> categories = get_all_category(db);
            for (const auto& category : categories) {
                if (category.second == newCategoryName.toStdString()) {
                    categoryID = category.first;
                    break;
                }
            }

            if (categoryID != -1)
            {
                move_app_to_category(db, appName.toStdString(), categoryID);
                initTreeWidget();
            }
            else
            {
                QMessageBox::critical(this, "Error", "Category does not exist！");
                onAppItemDoubleClicked(item, column);
            }
        }
    }
}

//展示统计时间
//补零操作
std::string add_leading_zero(int num)
{
    if (num < 10) {
        return "0" + std::to_string(num);
    } else {
        return std::to_string(num);
    }
}
//日期转换
int getdate(QDate date){
    int year = date.year();
    int month = date.month();
    int day = date.day();
    std::string y = add_leading_zero(year);
    std::string m = add_leading_zero(month);
    std::string d = std::to_string(day);
    std::string ymd = y + m + d;
    return std::stoi(ymd);
}

//创立统计
void MainWindow::createtimesum(){
    QDateEdit* datestart=ui->dateEdit;
    QDateEdit* dateend=ui->dateEdit_2;
    datestart->setDate(QDate::currentDate());
    dateend->setDate(QDate::currentDate());
    QTreeWidget* timesum = ui->time;
    timesum->header()->setSectionResizeMode(QHeaderView::Stretch);
    timesum->clear(); // 清空QTreeWidget中的所有项
    QDate startdate = datestart->date();
    QDate enddate = dateend->date();
    int start=getdate(startdate)*100,end=getdate(enddate)*100+99;
    timesum->setHeaderLabels(QStringList() << "appName" << "usageTime(min)");
    std::vector<std::pair<std::string, int>> appUsage = get_usage_app_timespan(db, start, end);
    for (const auto& app : appUsage)
    {
        std::string appName = app.first;
        int usageTime = app.second;
        QTreeWidgetItem* item = new QTreeWidgetItem(timesum);
        item->setText(0, QString::fromStdString(appName));
        item->setText(1, QString::number(usageTime));
    }
}

//改start time
void MainWindow::on_dateEdit_userDateChanged(const QDate &date)
{
    QDateEdit* datestart=ui->dateEdit;
    QDateEdit* dateend=ui->dateEdit_2;
    QTreeWidget* timesum = ui->time;
    timesum->header()->setSectionResizeMode(QHeaderView::Stretch);
    timesum->clear(); // 清空QTreeWidget中的所有项
    QDate startdate = datestart->date();
    QDate enddate = dateend->date();
    int start=getdate(startdate)*100,end=getdate(enddate)*100+99;
    timesum->setHeaderLabels(QStringList() << "appName" << "usageTime");
    std::vector<std::pair<std::string, int>> appUsage = get_usage_app_timespan(db, start, end);
    for (const auto& app : appUsage)
    {
        std::string appName = app.first;
        int usageTime = app.second;
        QTreeWidgetItem* item = new QTreeWidgetItem(timesum);
        item->setText(0, QString::fromStdString(appName));
        item->setText(1, QString::number(usageTime));
    }
}

//改end time
void MainWindow::on_dateEdit_2_userDateChanged(const QDate &date)
{
    QDateEdit* datestart=ui->dateEdit;
    QDateEdit* dateend=ui->dateEdit_2;
    QTreeWidget* timesum = ui->time;
    timesum->header()->setSectionResizeMode(QHeaderView::Stretch);
    timesum->clear(); // 清空QTreeWidget中的所有项
    QDate startdate = datestart->date();
    QDate enddate = dateend->date();
    int start=getdate(startdate)*100,end=getdate(enddate)*100+99;
    timesum->setHeaderLabels(QStringList() << "appName" << "usageTime");
    std::vector<std::pair<std::string, int>> appUsage = get_usage_app_timespan(db, start, end);
    for (const auto& app : appUsage)
    {
        std::string appName = app.first;
        int usageTime = app.second;
        QTreeWidgetItem* item = new QTreeWidgetItem(timesum);
        item->setText(0, QString::fromStdString(appName));
        item->setText(1, QString::number(usageTime));
    }
}

