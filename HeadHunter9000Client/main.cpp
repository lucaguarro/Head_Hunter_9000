#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    a.setStyleSheet(
        "QCheckBox { color: blue; }"
        "QCheckBox:disabled { color: grey; }"
    );
    return a.exec();
}
