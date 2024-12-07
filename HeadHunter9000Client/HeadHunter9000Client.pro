QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    askquestionsui.cpp \
    checkablecombobox.cpp \
    databasemanager.cpp \
    joblistingsui.cpp \
    jobpreviewwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    processworker.cpp \
    scraperconfigurationui.cpp \
    seeallquestionsui.cpp \
    sidebarjoblistwidget.cpp \
    starratingwidget.cpp

HEADERS += \
    askquestionsui.h \
    checkablecombobox.h \
    databasemanager.h \
    job.h \
    joblistingsui.h \
    jobpreviewwidget.h \
    mainwindow.h \
    processworker.h \
    scraperconfigurationui.h \
    seeallquestionsui.h \
    sidebarjoblistwidget.h \
    starratingwidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    assets/icons/arrow_left.svg \
    assets/icons/arrow_right.svg \
    assets/icons/database.svg \
    assets/icons/person_raised_hand.svg \
    assets/icons/robot.svg \
    assets/icons/star-regular.svg \
    assets/icons/star-solid.svg \
    assets/icons/work.svg

RESOURCES += \
    resources.qrc
