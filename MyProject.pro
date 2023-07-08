QT += core
QT       += core gui
QT       +=sql
QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
LIBS += -L"D:\bin\sqlite3" -lsqlite3

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    myaddreminder.cpp \
    utils.cpp

HEADERS += \
    mainwindow.h \
    myaddreminder.h \
    mywidget.h \
    sqlite3ext.h \
    utils.h

FORMS += \
    mainwindow.ui \
    myaddreminder.ui 

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../Desktop/QT_23proj-main/QT_23proj-main/.gitignore \
    ../../Desktop/QT_23proj-main/QT_23proj-main/data.db \
    ../../Desktop/QT_23proj-main/QT_23proj-main/readme.md \
    .gitignore \
    CMakeLists.txt.user \
    MyProject.pro.user \
    data.db \
    sqlite-dll-win64-x64-3420000/sqlite3.def \
    sqlite-dll-win64-x64-3420000/sqlite3.dll \
    utils.o
