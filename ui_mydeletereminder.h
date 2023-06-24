/********************************************************************************
** Form generated from reading UI file 'mydeletereminder.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYDELETEREMINDER_H
#define UI_MYDELETEREMINDER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MyDeleteReminder
{
public:
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QLabel *label;

    void setupUi(QWidget *MyDeleteReminder)
    {
        if (MyDeleteReminder->objectName().isEmpty())
            MyDeleteReminder->setObjectName("MyDeleteReminder");
        MyDeleteReminder->resize(351, 141);
        pushButton = new QPushButton(MyDeleteReminder);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(50, 80, 80, 18));
        pushButton_2 = new QPushButton(MyDeleteReminder);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(210, 80, 80, 18));
        label = new QLabel(MyDeleteReminder);
        label->setObjectName("label");
        label->setGeometry(QRect(100, 30, 181, 16));
        QFont font;
        font.setPointSize(10);
        label->setFont(font);

        retranslateUi(MyDeleteReminder);

        QMetaObject::connectSlotsByName(MyDeleteReminder);
    } // setupUi

    void retranslateUi(QWidget *MyDeleteReminder)
    {
        MyDeleteReminder->setWindowTitle(QCoreApplication::translate("MyDeleteReminder", "MyDeleteReminder", nullptr));
        pushButton->setText(QCoreApplication::translate("MyDeleteReminder", "\347\241\256\345\256\232", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MyDeleteReminder", "\345\217\226\346\266\210", nullptr));
        label->setText(QCoreApplication::translate("MyDeleteReminder", "\347\241\256\345\256\232\350\246\201\345\210\240\351\231\244\350\277\231\344\270\252\346\217\220\351\206\222\345\220\227\357\274\237", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MyDeleteReminder: public Ui_MyDeleteReminder {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYDELETEREMINDER_H
