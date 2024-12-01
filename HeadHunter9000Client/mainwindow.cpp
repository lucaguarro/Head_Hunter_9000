#include "mainwindow.h"
#include "processworker.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , previousButton(nullptr)
{
    ui->setupUi(this);

    // Determine the configuration file path
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.ini";

    // Ensure the directory exists
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    // Initialize QSettings with the config file path
    settings = new QSettings(configFilePath, QSettings::IniFormat, this);

    // Initialize the database manager with the settings instance
    dbManager = new DatabaseManager(settings);

    // SidebarMenu is at index 0, scrollArea is at index 1 in the splitter
    ui->splitter->setStretchFactor(0, 0);  // SidebarMenu gets no stretch (fixed size)
    ui->splitter->setStretchFactor(1, 1);  // scrollArea stretches to fill the remaining space
    ui->splitter->setSizes({200,1});

    QList<QPushButton*> sidebar_buttons = {ui->AnswerQuestionsBtn, ui->ScraperConfigBtn, ui->SeeAllQuestionsBtn, ui->ViewJobListingsBtn};
    for (auto button : sidebar_buttons) {
        connect(button, &QPushButton::clicked, this, [=]() {
            onSidebarButtonClicked(button, sidebar_buttons);
        });
    }
}

void MainWindow::cleanUpJobListingsPage() {
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
            delete seeallquestionsui;
        } else if (previousButton == ui->ScraperConfigBtn) {
            delete scraperconfigurationui;
        } else if (previousButton == ui->ViewJobListingsBtn) {
            cleanUpJobListingsPage();
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

void MainWindow::on_ScraperConfigBtn_clicked()
{
    scraperconfigurationui = new ScraperConfigurationUI(this, settings);
    connect(scraperconfigurationui, &ScraperConfigurationUI::databasePathChanged, this, &MainWindow::onDatabasePathChanged);
    ui->mainAreaContainer->layout()->addWidget(scraperconfigurationui);
}

void MainWindow::onDatabasePathChanged() {
    dbManager->setDatabasePath();
}

void MainWindow::setExecutionStateUI() {
    if (isProcessRunning){
        ui->ExecuteBtn->setText("Stop Execution");
        ui->applyModeCheckbox->setDisabled(true);
    } else {
        ui->ExecuteBtn->setText("Execute");
        ui->applyModeCheckbox->setDisabled(false);
    }
}

void MainWindow::on_ExecuteBtn_clicked() {
    if (!isProcessRunning) {
        // Start the process
        QString scriptPath = "/home/luca/Documents/Projects/Head_Hunter_9000/run_scraper.sh";
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.ini";

        thread = new QThread;
        worker = new ProcessWorker("/bin/bash", QStringList() << scriptPath << configPath << QString::number(ui->applyModeCheckbox->isChecked()));

        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &ProcessWorker::execute);
        connect(worker, &ProcessWorker::processFinished, this, [this](const QString& output, const QString& errorOutput) {
            qDebug() << "Script Output:" << output;
            if (!errorOutput.isEmpty()) {
                qDebug() << "Script Error:" << errorOutput;
            }
            // Process finished, reset state
            isProcessRunning = false;
            setExecutionStateUI();

        });
        connect(worker, &ProcessWorker::processError, this, [this](const QString& errorMessage) {
            qDebug() << errorMessage;
            // Process error, reset state
            isProcessRunning = false;
            setExecutionStateUI();
        });

        connect(worker, &ProcessWorker::processFinished, thread, &QThread::quit);
        connect(worker, &ProcessWorker::processFinished, worker, &ProcessWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        // Reset worker and thread pointers when thread finishes
        connect(thread, &QThread::finished, this, [this]() {
            worker = nullptr;
            thread = nullptr;
        });

        isProcessRunning = true;
        setExecutionStateUI();

        thread->start();

    } else {
        // Stop the process
        if (worker) {
            // Emit signal to stop the process
            QMetaObject::invokeMethod(worker, "stop", Qt::QueuedConnection);
        }
        // The state will be reset when the process finishes
    }
}


void MainWindow::on_SeeAllQuestionsBtn_clicked()
{
    seeallquestionsui = new SeeAllQuestionsUI(this, this->dbManager);
    ui->mainAreaContainer->layout()->addWidget(seeallquestionsui);
}


void MainWindow::on_AnswerQuestionsBtn_clicked()
{
    askquestionsui = new AskQuestionsUI(this, this->dbManager);
    ui->mainAreaContainer->layout()->addWidget(askquestionsui);
}

void MainWindow::on_ViewJobListingsBtn_clicked(){
    joblistingsui = new JobListingsUI(this, this->dbManager);
    ui->mainAreaContainer->layout()->addWidget(joblistingsui);
}
