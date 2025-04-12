#include "preferencesdialogui.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>

PreferencesDialogUI::PreferencesDialogUI(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Preferences");
    resize(600, 400);
    setupUI();
}

void PreferencesDialogUI::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    sidePanel = new QListWidget(this);
    sidePanel->addItem("General");
    sidePanel->addItem("Appearance");
    sidePanel->addItem("Network");
    sidePanel->setFixedWidth(150);

    stackedWidget = new QStackedWidget(this);

    createPages();

    connect(sidePanel, &QListWidget::currentRowChanged,
            stackedWidget, &QStackedWidget::setCurrentIndex);

    sidePanel->setCurrentRow(0);

    mainLayout->addWidget(sidePanel);
    mainLayout->addWidget(stackedWidget);
}

void PreferencesDialogUI::createPages() {
    QWidget *generalPage = new QWidget(this);
    QVBoxLayout *generalLayout = new QVBoxLayout(generalPage);
    generalLayout->addWidget(new QLabel("General Settings", generalPage));

    QWidget *appearancePage = new QWidget(this);
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearancePage);
    appearanceLayout->addWidget(new QLabel("Appearance Settings", appearancePage));

    QWidget *networkPage = new QWidget(this);
    QVBoxLayout *networkLayout = new QVBoxLayout(networkPage);
    networkLayout->addWidget(new QLabel("Network Settings", networkPage));

    stackedWidget->addWidget(generalPage);
    stackedWidget->addWidget(appearancePage);
    stackedWidget->addWidget(networkPage);
}
