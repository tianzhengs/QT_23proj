#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlError>
#include <QInputDialog>
#include <QMessageBox>
#include <utils.h>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

MainWindow::MainWindow(sqlite3* connection, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db(connection),newreminder(db),newdeletereminder(db)
{
    ui->setupUi(this);
    treeWidget = ui->CategoryList;
    timelineView = ui->graphicsView;
    // Set up the tree widget
    treeWidget->setColumnCount(1); // Set the number of columns
    treeWidget->setHeaderLabels({ "Categories" }); // Set the column headers
    // Get the categories from the database
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);

    // Iterate through the categories and add them to the QTreeWidget
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

}

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



MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()//添加新的category
{
    //跳转新页面
    // Open an input dialog to get the new category name
    QString categoryName = QInputDialog::getText(this, "Add Category", "Enter Category Name");

    // Check if the user entered a valid category name
    if (!categoryName.isEmpty())
    {
        // Add the category to the tree widget
        add_category(db,categoryName.toStdString());
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(treeWidget);
        categoryItem->setText(0, categoryName);
    }

}


void MainWindow::on_pushButton_2_clicked()//delete item
{
    QTreeWidgetItem *selectedItem = treeWidget->currentItem();
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);
    // 获取选定项目的类别名称
    QString categoryText = selectedItem->text(0); // 将column替换为对应类别名称的列索引

    // 查找类别ID
    int categoryID = -1; // 默认值为-1，表示未找到对应的类别ID
    for (const auto& category : categories) {
        if (category.second == categoryText.toStdString()) {
            categoryID = category.first;
            break;
        }
    }
    if (selectedItem)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmation", "Are you sure you want to delete this item?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            if (delete_category(db,categoryID))
            {
                // 删除选中的项

                delete selectedItem;
            }
            else
            {
                // 删除失败，处理错误情况
            }
        }
    }
}

void MainWindow::initTreeWidget()//导入现有
{
    // Set up the tree widget
    treeWidget->setColumnCount(1); // Set the number of columns
    treeWidget->setHeaderLabels(QStringList()<<QString::fromLocal8Bit("Category Name"));

    // Get the categories from the database
    std::vector<std::pair<int, std::string>> categories = get_all_category(db);
    treeWidget->clear();


    // Iterate through the categories and add them to the QTreeWidget
    for (const auto& category : categories)
    {
        // Create a category item
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(treeWidget);
        categoryItem->setText(0, QString::fromStdString(category.second));

        // Get the apps under the current category
        std::vector<std::string> appNames = get_apps_under_category(db, category.first);

        // Add each app as a child item under the current category
        for (const std::string& appName : appNames)
        {
            QTreeWidgetItem* appItem = new QTreeWidgetItem(categoryItem);
            appItem->setFlags(appItem->flags() | Qt::ItemIsDragEnabled);
            appItem->setText(0, QString::fromStdString(appName));
        }
    }
}

void MainWindow::on_pushButton_reminder1_clicked()
{
    newreminder.show();
    displayReminders();
}


void MainWindow::on_pushButton_reminder3_clicked()
{
    displayReminders();
}


void MainWindow::on_pushButton_reminder2_clicked()
{
     newdeletereminder.show();
}




void MainWindow::on_ReminderList_clicked(const QModelIndex &index0)
{
    int row;
    row=index0.row();
    auto index1=ui->ReminderList->topLevelItem(row);
    newdeletereminder.delete_name=(index1->text(0)).toStdString();
    ui->pushButton_reminder2->show();
}

void MainWindow::on_CategoryList_doubleClicked(const QModelIndex &index)
{
    QTreeWidgetItem* selectedItem = treeWidget->currentItem(); // 获取选定的项目
    int column = index.column(); // 获取双击的列索引
    if (selectedItem && column == 0 && !selectedItem->parent()) // 仅在第一列双击且没有父项时触发重命名操作
    {
        std::vector<std::pair<int, std::string>> categories = get_all_category(db);
        // 获取选定项目的类别名称
        QString categoryText = selectedItem->text(0); // 将column替换为对应类别名称的列索引

        // 查找类别ID
        int categoryID = -1; // 默认值为-1，表示未找到对应的类别ID
        for (const auto& category : categories) {
            if (category.second == categoryText.toStdString()) {
                categoryID = category.first;
                break;
            }
        }
        QString currentName = selectedItem->text(0); // 获取当前名称
        QString newName = QInputDialog::getText(this, "重命名", "输入新名称", QLineEdit::Normal, currentName);
        if (!newName.isEmpty() && newName != currentName)
        {
            std::string new_name = newName.toStdString();
            if (rename_category(db, categoryID, new_name))
            {
                selectedItem->setText(0, newName); // 更新显示的名称
            }
            else
            {
                QMessageBox::critical(this, "错误", "重命名失败");
            }
        }
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->acceptProposedAction();
    }
}
// 添加拖拽事件处理函数
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasFormat("application/x-qabstractitemmodeldatalist")) {
        QByteArray encodedData = mimeData->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            int row, col;
            QMap<int, QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;

            // 获取拖拽的项的名称和目标类别
            QString appName = roleDataMap[Qt::DisplayRole].toString();
            QString categoryName = treeWidget->itemAt(event->position().toPoint())->text(0);

            // 获取目标类别ID
            int newCategoryId = -1;
            std::vector<std::pair<int, std::string>> categories = get_all_category(db);
            for (const auto& category : categories) {
                if (QString::fromStdString(category.second) == categoryName) {
                    newCategoryId = category.first;
                    break;
                }
            }

            // 执行移动操作
            if (newCategoryId != -1 && move_app_to_category(db, appName.toStdString(), newCategoryId)) {
                // 移除原来的项
                delete treeWidget->itemFromIndex(treeWidget->currentIndex());
            }
        }

        // 刷新页面
        initTreeWidget();
        event->acceptProposedAction();
    }
}
void MainWindow::createTimeline()
{
    // Create the QGraphicsView and QGraphicsScene

    timelineView->setScene(timelineScene);

    // Add the QGraphicsView to your layout or main window

    // Get the app usage timeline data
    std::vector<std::pair<int, std::vector<std::pair<std::string, int>>>> timelineData = get_app_usage_timeline_day(db);

    // Clear the scene
    timelineScene->clear();

    // Set the timeline height and position
    int timelineHeight = 50;
    int timelinePositionY = 100;

    // Set the width and position of each time interval on the timeline
    int timeIntervalWidth = 50;
    int timeIntervalPositionX = 0;

    // Iterate over each time interval in the timeline data
    for (const auto& timeInterval : timelineData)
    {
        int hour = timeInterval.first;
        const std::vector<std::pair<std::string, int>>& appUsage = timeInterval.second;

        // Add a label for the hour
        QGraphicsTextItem* hourLabel = new QGraphicsTextItem(QString::number(hour));
        hourLabel->setPos(timeIntervalPositionX, timelinePositionY - 20);
        timelineScene->addItem(hourLabel);

        // Iterate over each app usage in the current time interval
        for (const auto& app : appUsage)
        {
            std::string appName = app.first;
            int usageMinutes = app.second;

            // Calculate the height of the app bar based on the usage time
            int appHeight = usageMinutes * 2;

            // Create a QGraphicsRectItem representing the app bar
            QGraphicsRectItem* appBarItem = new QGraphicsRectItem();
            appBarItem->setRect(timeIntervalPositionX, timelinePositionY, timeIntervalWidth, appHeight);
            appBarItem->setBrush(Qt::blue);
            timelineScene->addItem(appBarItem);

            // Add a label for the app name
            QGraphicsTextItem* appNameLabel = new QGraphicsTextItem(QString::fromStdString(appName));
            appNameLabel->setPos(timeIntervalPositionX, timelinePositionY + appHeight + 5);
            timelineScene->addItem(appNameLabel);
        }

    // Update the position for the next time interval
    }
}
