QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    askquestionsui.cpp \
    checkablecombobox.cpp \
    databasemanager.cpp \
    main.cpp \
    mainwindow.cpp \
    seeallquestionsui.cpp

HEADERS += \
    askquestionsui.h \
    checkablecombobox.h \
    databasemanager.h \
    mainwindow.h \
    seeallquestionsui.h

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
    assets/icons/work.svg

RESOURCES += \
    resources.qrc
