/********************************************************************************
** Form generated from reading UI file 'myaddreminder.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYADDREMINDER_H
#define UI_MYADDREMINDER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MyAddReminder
{
public:
    QPushButton *pushButton;
    QLabel *label;
    QPushButton *pushButton_2;
    QLineEdit *lineEdit;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QLineEdit *lineEdit_4;

    void setupUi(QWidget *MyAddReminder)
    {
        if (MyAddReminder->objectName().isEmpty())
            MyAddReminder->setObjectName("MyAddReminder");
        MyAddReminder->resize(394, 231);
        QFont font;
        font.setPointSize(7);
        MyAddReminder->setFont(font);
        pushButton = new QPushButton(MyAddReminder);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(280, 90, 80, 21));
        label = new QLabel(MyAddReminder);
        label->setObjectName("label");
        label->setGeometry(QRect(30, 50, 91, 16));
        label->setFont(font);
        pushButton_2 = new QPushButton(MyAddReminder);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(280, 50, 80, 21));
        lineEdit = new QLineEdit(MyAddReminder);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setGeometry(QRect(140, 50, 113, 22));
        label_2 = new QLabel(MyAddReminder);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(30, 90, 120, 16));
        label_2->setFont(font);
        label_3 = new QLabel(MyAddReminder);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(30, 130, 120, 12));
        label_3->setFont(font);
        label_4 = new QLabel(MyAddReminder);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(30, 170, 120, 12));
        label_4->setFont(font);
        lineEdit_2 = new QLineEdit(MyAddReminder);
        lineEdit_2->setObjectName("lineEdit_2");
        lineEdit_2->setGeometry(QRect(140, 90, 113, 20));
        lineEdit_3 = new QLineEdit(MyAddReminder);
        lineEdit_3->setObjectName("lineEdit_3");
        lineEdit_3->setGeometry(QRect(140, 130, 113, 20));
        lineEdit_4 = new QLineEdit(MyAddReminder);
        lineEdit_4->setObjectName("lineEdit_4");
        lineEdit_4->setGeometry(QRect(140, 170, 113, 20));

        retranslateUi(MyAddReminder);

        QMetaObject::connectSlotsByName(MyAddReminder);
    } // setupUi

    void retranslateUi(QWidget *MyAddReminder)
    {
        MyAddReminder->setWindowTitle(QCoreApplication::translate("MyAddReminder", "Form", nullptr));
#if QT_CONFIG(tooltip)
        MyAddReminder->setToolTip(QCoreApplication::translate("MyAddReminder", "\346\267\273\345\212\240\346\226\260\345\210\206\347\273\204", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton->setText(QCoreApplication::translate("MyAddReminder", "return", nullptr));
        label->setText(QCoreApplication::translate("MyAddReminder", "Reminder Name", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MyAddReminder", "\346\267\273\345\212\240", nullptr));
        lineEdit->setPlaceholderText(QCoreApplication::translate("MyAddReminder", "\350\257\267\350\276\223\345\205\245", nullptr));
        label_2->setText(QCoreApplication::translate("MyAddReminder", "Reminder BindCategory", nullptr));
        label_3->setText(QCoreApplication::translate("MyAddReminder", "Reminder TimeLimit", nullptr));
        label_4->setText(QCoreApplication::translate("MyAddReminder", "Reminder Message", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MyAddReminder: public Ui_MyAddReminder {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYADDREMINDER_H
