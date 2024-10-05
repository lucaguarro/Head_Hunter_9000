#include "askquestionsui.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , previousButton(nullptr)
{
    ui->setupUi(this);

    // Initialize the database manager with the path to the SQLite file
    dbManager = new DatabaseManager("/home/luca/Documents/Projects/Head_Hunter_9000/example.db");

    // SidebarMenu is at index 0, scrollArea is at index 1 in the splitter
    ui->splitter->setStretchFactor(0, 0);  // SidebarMenu gets no stretch (fixed size)
    ui->splitter->setStretchFactor(1, 1);  // scrollArea stretches to fill the remaining space
    ui->splitter->setSizes({200,1});

    QList<QPushButton*> sidebar_buttons = {ui->AnswerQuestionsBtn, ui->ScraperConfigBtn, ui->SeeAllQuestionsBtn, ui->JobSearchCriteriaBtn};
    for (auto button : sidebar_buttons) {
        connect(button, &QPushButton::clicked, this, [=]() {
            onSidebarButtonClicked(button, sidebar_buttons);
        });
    }
}

void MainWindow::cleanUpAnswerQuestionsPage() {
    qDebug() << "Ayo";
}

void MainWindow::cleanUpSeeAllQuestionsPage() {
    qDebug() << "Ayo2";
}

void MainWindow::cleanUpScraperConfigPage() {
    qDebug() << "Ayo3";
}

void MainWindow::cleanUpJobSearchCriteriaPage() {
    qDebug() << "Ayo4";
}

// Slot function to handle button click logic
void MainWindow::onSidebarButtonClicked(QPushButton* clickedButton, const QList<QPushButton*>& buttons) {
    // Check if there was a previously disabled button
    if (previousButton != nullptr) {
        // Perform UI cleanup based on the previously disabled button
        if (previousButton == ui->AnswerQuestionsBtn) {
            delete askquestionsui;
        } else if (previousButton == ui->SeeAllQuestionsBtn) {
            cleanUpSeeAllQuestionsPage();
        } else if (previousButton == ui->ScraperConfigBtn) {
            cleanUpScraperConfigPage();
        } else if (previousButton == ui->JobSearchCriteriaBtn) {
            cleanUpJobSearchCriteriaPage();
        }
    }

    // Loop through the buttons and enable/disable them appropriately
    for (auto button : buttons) {
        if (button == clickedButton) {
            button->setDisabled(true);  // Disable the clicked button
        } else {
            button->setEnabled(true);  // Enable all the other buttons
        }
    }

    // Store the clicked button as the newly disabled button
    previousButton = clickedButton;
}

MainWindow::~MainWindow() {
    delete ui;
    delete dbManager;
}


void MainWindow::on_AnswerQuestionsBtn_clicked()
{
    askquestionsui = new AskQuestionsUI(this, this->dbManager);
    ui->mainAreaContainer->layout()->addWidget(askquestionsui);
}
