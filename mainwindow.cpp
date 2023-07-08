#include "mainwindow.h"
#include "myaddreminder.h"
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
#include "mywidget.h"
#include <QPainter>
#include <QRect>
#include <QVBoxLayout>
#include <QLabel>
#include <QFontDialog>
#include <QRegularExpression>


MainWindow::MainWindow(sqlite3* connection, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db(connection),newreminder(db)
{
    QObject::connect(&newreminder, &MyAddReminder::messageReceived, this, &MainWindow::handleWindowClosed);
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
    timesumView =ui->graph;
    timesumScene = new QGraphicsScene(this);
    timesumView->setScene(timesumScene);
    createtimesum();
    init();
    QString menuWidgetStyle = "background-color: white;";
    // 创建按钮
    QPushButton* menuButton = new QPushButton("设置", this);

    // 创建菜单
    QMenu* menu = new QMenu(this);

    // 创建操作
    QAction* action1 = new QAction("颜色", this);
    QAction* action2 = new QAction("字体", this);
    QAction* action3 = new QAction("图片", this);

    // 将操作添加到菜单中
    menu->addAction(action1);
    menu->addAction(action2);
    menu->addAction(action3);

    // 将菜单设置为菜单按钮的下拉菜单
    menuButton->setMenu(menu);
    menuButton->setStyleSheet(menuWidgetStyle);
    menu->setStyleSheet(menuWidgetStyle);

    menuButton->setGeometry(35, 0, 100, 25);
    // 将菜单操作的触发信号与槽函数进行连接
    connect(action1, &QAction::triggered, this, &MainWindow::handleAction1);
    connect(action2, &QAction::triggered, this, &MainWindow::handleAction2);
    connect(action3, &QAction::triggered, this, &MainWindow::handleAction3);
}


MainWindow::~MainWindow()
{
    delete ui;
}

//改背景颜色
void MainWindow::handleAction1()
{
    QColorDialog colorDialog;
    colorDialog.setWindowTitle("Choose a Color");
    colorDialog.setCurrentColor(Qt::red);  // 设置默认颜色
    QColor selectedColor = colorDialog.getColor();
    if (selectedColor.isValid()) {
        ui->centralwidget->setStyleSheet("background-color: " + selectedColor.name() + ";");
        ui->menubar->setStyleSheet("background-color: " + selectedColor.name() + ";");
        ui->statusbar->setStyleSheet("background-color: " + selectedColor.name() + ";");
    }
}

//改字体
void MainWindow::handleAction2()
{
    QFontDialog fontDialog;
    fontDialog.setWindowTitle("选择字体");
    fontDialog.setCurrentFont(QFont("Arial", 12));
    if (fontDialog.exec() == QDialog::Accepted) {
        QFont selectedFont = fontDialog.selectedFont();
        qApp->setFont(selectedFont);
    }
    QString styleSheet = ui->centralwidget->styleSheet();
    QRegularExpression regex("background-image: url\\((.*?)\\)");
    QRegularExpressionMatch match = regex.match(styleSheet);
    if (match.hasMatch()) {
        QString backgroundImagePath = match.captured(1);
        ui->centralwidget->setStyleSheet("background-image: url("+backgroundImagePath+"); background-repeat: no-repeat; background-position: center;");
        //qDebug() << "Background Image Path:" << backgroundImagePath;
    } else {
        // 没有设置背景图片
        QColor backgroundColor = ui->centralwidget->palette().color(QPalette::Window);
        //std::cerr<<std::(backgroundColor.name())<<std::endl;
        if(backgroundColor.alpha()!=0) ui->centralwidget->setStyleSheet("background-color: " + backgroundColor.name() + ";");
        else ui->centralwidget->setStyleSheet("background-color: transparent;");
    }
}

void MainWindow::handleAction3()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.jpeg)");

    if (!filePath.isEmpty()) {
        QWidget* centralWidget = ui->centralwidget;
        centralWidget->setStyleSheet("background-image: url("+filePath+"); background-repeat: no-repeat; background-position: center;");
        // 处理选中的图片文件路径
        // 将图片设置为背景或进行其他操作
    }
}

//设置其他部件的样式表，保持默认颜色
void MainWindow::init(){

    //std::cerr<<"initsuccess"<<std::endl;
    QList<QWidget*> allWidgets = this->findChildren<QWidget*>();
    QString otherWidgetStyle = "background-color: transparent;";
    for (QWidget* widget : allWidgets) {
        widget->setStyleSheet(otherWidgetStyle);  // 根据实际情况设置其他部件的样式表
    }
}

//---------------------------category--------------------------------------------
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

//-----------------------reminder------------------------------------------
//展示reminder
void MainWindow::displayReminders() {
    QTreeWidget* treeWidget = ui->ReminderList;
    treeWidget->clear(); // 清空QTreeWidget中的所有项

    // 从数据库获取所有的reminder
    std::vector<Reminder> reminders = get_all_reminder(db);

    // 遍历reminders并将其添加到QTreeWidget中
    for (Reminder reminder : reminders) {
        int Categoryid=reminder.reminderBindCategory;
        std::string reminderBindCategory;
        std::vector<std::pair<int, std::string>> categories = get_all_category(db);
        for(const auto& category : categories){
            if(category.first==Categoryid) reminderBindCategory=category.second;
        }
        QTreeWidgetItem* reminderItem = new QTreeWidgetItem(treeWidget);
        reminderItem->setText(0, QString::fromStdString(reminder.reminderName));
        reminderItem->setText(1, QString::fromStdString(reminderBindCategory));
        reminderItem->setText(2, QString::number(reminder.reminderTimeLimit));
        reminderItem->setText(3, QString::fromStdString(reminder.reminderMessage));
        if(reminder.reached()) reminderItem->setText(4,"yes");
        else reminderItem->setText(4,"no");
    }
    ui->pushButton_reminder2->hide();
}

//新增reminder
void MainWindow::on_pushButton_reminder1_clicked()
{
    newreminder.show();
    displayReminders();
}


void MainWindow::handleWindowClosed()
{
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
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Comfirmation", "Are you sure you want to delete this reminder?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            if (remove_reminder(db,delete_name))
            {
                displayReminders();
            }
            }
            else
            {
                QMessageBox::critical(this, "error", "deletion failed！");
            }
        }

//选中reminder
void MainWindow::on_ReminderList_clicked(const QModelIndex &index0)
{
    int row;
    row=index0.row();
    auto index1=ui->ReminderList->topLevelItem(row);
    delete_name=(index1->text(0)).toStdString();
    ui->pushButton_reminder2->show();
}

//连接槽函数
void MainWindow::handleMessage(std::string message)
{
    // 处理收到的消息，可以在这里进行UI操作或其他逻辑处理
    QMessageBox::information(this, "Message", QString::fromStdString(message));
}


//------------------------------时间轴---------------------------------------
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
    int maxPosition = 0;
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
            if(appHeight>maxPosition)
                maxPosition=appHeight;
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
    int legendPositionY = maxPosition+20+timelinePositionY;
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


//----------------------------------展示统计时间------------------------------------
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
    std::string d = add_leading_zero(day);
    std::string ymd = y + m + d;
    return std::stoi(ymd);
}


void MainWindow::drawBarChart(int start,int end)
{
    timesumScene->clear();
    std::vector<std::pair<std::string, int>> dataLabels = get_usage_app_timespan(db, start, end);

    int numBars = dataLabels.size();
    int barWidth = 30;
    int barSpacing = 70;
    int chartHeight = 100;
    int maxDataValue;
    if(numBars > 0){
        maxDataValue = std::max_element(dataLabels.begin(), dataLabels.end(),
                                        [](const auto& a, const auto& b) { return a.second < b.second; })->second;
    }
    else maxDataValue = 0;
    int totalWidth = numBars * (barWidth + barSpacing) - barSpacing;
    int startX = (timesumView->width() - totalWidth) / 2; // 计算柱状图起始 X 坐标

    int startY = (timesumView->height() - chartHeight) / 2; // 计算柱状图起始 Y 坐标

    // 绘制 x 轴线和描述文本
    int xAxisY = startY + chartHeight;
    int xAxisX1 = startX - barSpacing / 2;
    int xAxisX2 = startX + totalWidth + barSpacing / 2;
    timesumScene->addLine(xAxisX1, xAxisY, xAxisX2, xAxisY, QPen(Qt::black));

    QFont labelFont("Arial", 10);
    int labelOffsetY = 20; // 描述文本距离 x 轴的垂直偏移量

    for (int i = 0; i < numBars; ++i)
    {
        int dataValue = dataLabels[i].second;
        QString label = QString::fromStdString(dataLabels[i].first);

        int barHeight = (double)dataValue / maxDataValue * chartHeight;

        QRectF rect(startX, startY + chartHeight - barHeight, barWidth, barHeight);
        QGraphicsRectItem* barItem = new QGraphicsRectItem(rect);
        barItem->setBrush(Qt::blue);
        timesumScene->addItem(barItem);

        // 绘制 x 轴描述文本
        QGraphicsTextItem* labelItem = new QGraphicsTextItem(label);
        labelItem->setFont(labelFont);
        labelItem->setPos(startX + barWidth / 2-27, xAxisY + labelOffsetY);
        QString labelText = QString::fromStdString(dataLabels[i].first);
        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        labelItem->setPlainText(labelText);
        labelItem->setTextWidth(95);
        labelItem->document()->setDefaultTextOption(textOption);
        timesumScene->addItem(labelItem);
        startX += barWidth + barSpacing;
        //std::cerr<<dataLabels[i].first<<" "<<startX<<std::endl;

    }
}


//创立统计
void MainWindow::createtimesum(){
    QDateEdit* datestart=ui->dateEdit;
    QDateEdit* dateend=ui->dateEdit_2;
    dateend->setDate(QDate::currentDate());
    datestart->setDate(QDate::currentDate());
    QTreeWidget* timesum = ui->time;
    timesum->header()->setSectionResizeMode(QHeaderView::Stretch);
    timesum->clear(); // 清空QTreeWidget中的所有项
    QDate startdate = datestart->date();
    QDate enddate = dateend->date();
    int start=getdate(startdate)*100,end=getdate(enddate)*100+24;
    timesum->setHeaderLabels(QStringList() << "appName" << "usageTime(min)");
    std::vector<std::pair<std::string, int>> appUsage = get_usage_app_timespan(db, start, end);
    //std::cerr<<sizeof(appUsage)<<std::endl;
    for (const auto& app : appUsage)
    {
        //std::cerr<<"输出成功"<<std::endl;
        std::string appName = app.first;
        int usageTime = app.second;
        QTreeWidgetItem* item = new QTreeWidgetItem(timesum);
        item->setText(0, QString::fromStdString(appName));
        item->setText(1, QString::number(usageTime));
    }
    drawBarChart(start,end);
}

//改start time
void MainWindow::on_dateEdit_userDateChanged(const QDate &date)
{
    QDateEdit* datestart=ui->dateEdit;
    QDateEdit* dateend=ui->dateEdit_2;
    QTreeWidget* timesum = ui->time;
    timesum->header()->setSectionResizeMode(QHeaderView::Stretch);
    QDate startdate = datestart->date();
    QDate enddate = dateend->date();
    if(startdate>enddate){
        startdate=enddate;
        datestart->setDate(enddate);
    }//开始时间不能小于结束时间
    int start=getdate(startdate)*100,end=getdate(enddate)*100+24;
    timesum->setHeaderLabels(QStringList() << "appName" << "usageTime");
    //std::cerr<<start<<" "<<end<<std::endl;
    std::vector<std::pair<std::string, int>> appUsage = get_usage_app_timespan(db, start, end);
    timesum->clear(); // 清空QTreeWidget中的所有项
    //std::cerr<<"---------------"<<std::endl;
    for (const auto& app : appUsage)
    {
        std::string appName = app.first;
        int usageTime = app.second;
        QTreeWidgetItem* item = new QTreeWidgetItem(timesum);
        item->setText(0, QString::fromStdString(appName));
        item->setText(1, QString::number(usageTime));
        //std::cerr<<appName<<" "<<usageTime<<std::endl;
    }
    drawBarChart(start,end);
    //std::cerr<<"----------------"<<std::endl;
}

//改end time
void MainWindow::on_dateEdit_2_userDateChanged(const QDate &date)
{
    QDateEdit* datestart=ui->dateEdit;
    QDateEdit* dateend=ui->dateEdit_2;
    QTreeWidget* timesum = ui->time;
    timesum->header()->setSectionResizeMode(QHeaderView::Stretch);
    QDate startdate = datestart->date();
    QDate enddate = dateend->date();
    if(enddate>QDate::currentDate()){
        dateend->setDate(QDate::currentDate());
        enddate=QDate::currentDate();
    }
    if(startdate>enddate){
        //std::cerr<<"chenggong"<<std::endl;
        datestart->setDate(enddate);
        startdate=enddate;
    }
    int start=getdate(startdate)*100,end=getdate(enddate)*100+24;
    timesum->setHeaderLabels(QStringList() << "appName" << "usageTime");
    std::vector<std::pair<std::string, int>> appUsage = get_usage_app_timespan(db, start, end);
    timesum->clear(); // 清空QTreeWidget中的所有项
    //std::cerr<<sizeof(appUsage)<<std::endl;
    for (const auto& app : appUsage)
    {
        //std::cerr<<"输出成功"<<std::endl;
        std::string appName = app.first;
        int usageTime = app.second;
        QTreeWidgetItem* item = new QTreeWidgetItem(timesum);
        item->setText(0, QString::fromStdString(appName));
        item->setText(1, QString::number(usageTime));
    }
    drawBarChart(start,end);
}





