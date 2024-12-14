#include "mainwindow.h"
#include "ui_mainwindow.h"

// UI classes
#include "processworker.h"
#include "sidebarjoblistwidget.h"
#include "joblistingsui.h"
#include "askquestionsui.h"
#include "seeallquestionsui.h"
#include "scraperconfigurationui.h"

#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dbManager(nullptr)
    , sidebarjoblistwidget(nullptr)
    , settings(nullptr)
    , thread(nullptr)
    , worker(nullptr)
    , isProcessRunning(false)
    , currentWidget(nullptr)
    , previousButton(nullptr)
{
    ui->setupUi(this);

    // Determine the configuration file path
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.ini";
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)); // Ensure dir exists

    // Initialize QSettings and DatabaseManager
    settings = new QSettings(configFilePath, QSettings::IniFormat, this);
    dbManager = new DatabaseManager(settings);

    // Configure splitter
    ui->splitter->setStretchFactor(0, 0); // Sidebar fixed
    ui->splitter->setStretchFactor(1, 1); // scrollArea stretches
    ui->splitter->setSizes({200, 1});

    // Collect relevant sidebar buttons
    QList<QPushButton*> sidebar_buttons = {
        ui->AnswerQuestionsBtn,
        ui->ScraperConfigBtn,
        ui->SeeAllQuestionsBtn
    };
    for (auto button : sidebar_buttons) {
        connect(button, &QPushButton::clicked, this, [=]() {
            onSidebarButtonClicked(button, sidebar_buttons);
        });
    }

    // Create the sidebar job list widget
    sidebarjoblistwidget = new SidebarJobListWidget(dbManager, this);

    // Insert sidebarjoblistwidget below ScraperConfigBtn in the sidebar
    QWidget *sidebarMenu = findChild<QWidget *>("SidebarMenu");
    QVBoxLayout *sidebarLayout = qobject_cast<QVBoxLayout *>(sidebarMenu->layout());
    QPushButton *ScraperConfigBtn = findChild<QPushButton *>("ScraperConfigBtn");
    int insertIndex = sidebarLayout->indexOf(ScraperConfigBtn) + 1;
    sidebarLayout->insertWidget(insertIndex, sidebarjoblistwidget);

    // Connect the jobListingRequested signal
    connect(sidebarjoblistwidget, &SidebarJobListWidget::jobListingRequested, this, &MainWindow::createJobListingsUI);
}

MainWindow::~MainWindow()
{
    // Delete current widget if it exists
    if (currentWidget) {
        delete currentWidget;
        currentWidget = nullptr;
    }

    delete ui;
    delete dbManager;
    // All children of MainWindow, including sidebarjoblistwidget, are automatically deleted
}

//---------------------------------------------------------------------------------
// Utility function: Replaces whatever is in mainAreaContainer with newWidget
//---------------------------------------------------------------------------------
void MainWindow::setMainWidget(QWidget *newWidget)
{
    if (!ui->mainAreaContainer->layout()) {
        // If mainAreaContainer has no layout yet, create one
        QVBoxLayout *vLayout = new QVBoxLayout(ui->mainAreaContainer);
        ui->mainAreaContainer->setLayout(vLayout);
    }

    QLayout *layout = ui->mainAreaContainer->layout();

    // Remove and delete the old widget if it exists
    if (currentWidget) {
        layout->removeWidget(currentWidget);
        delete currentWidget;
        currentWidget = nullptr;
    }

    // Add the new widget
    if (newWidget) {
        layout->addWidget(newWidget);
        currentWidget = newWidget;
    }
}

//---------------------------------------------------------------------------------
// Sidebar button click handling
//---------------------------------------------------------------------------------
void MainWindow::onSidebarButtonClicked(QPushButton *clickedButton, const QList<QPushButton *> &buttons)
{
    // If there was a previously disabled button, re-enable it
    if (previousButton != nullptr) {
        previousButton->setEnabled(true);
    }

    // Disable the newly clicked button
    clickedButton->setEnabled(false);
    previousButton = clickedButton;
}

//---------------------------------------------------------------------------------
// SCRAPER CONFIG
//---------------------------------------------------------------------------------
void MainWindow::on_ScraperConfigBtn_clicked()
{
    ScraperConfigurationUI *scraperconfigurationui = new ScraperConfigurationUI(this, settings);
    connect(scraperconfigurationui, &ScraperConfigurationUI::databasePathChanged, this, &MainWindow::onDatabasePathChanged);
    setMainWidget(scraperconfigurationui);
}

void MainWindow::onDatabasePathChanged()
{
    dbManager->setDatabasePath();
}

//---------------------------------------------------------------------------------
// VIEW ALL QUESTIONS
//---------------------------------------------------------------------------------
void MainWindow::on_SeeAllQuestionsBtn_clicked()
{
    SeeAllQuestionsUI *seeallquestionsui = new SeeAllQuestionsUI(this, dbManager);
    setMainWidget(seeallquestionsui);
}

//---------------------------------------------------------------------------------
// ANSWER QUESTIONS
//---------------------------------------------------------------------------------
void MainWindow::on_AnswerQuestionsBtn_clicked()
{
    AskQuestionsUI *askquestionsui = new AskQuestionsUI(this, dbManager);
    setMainWidget(askquestionsui);
}

void MainWindow::createJobListingsUI()
{
    // If we call createJobListingsUI() repeatedly,
    // a new widget will be created each time. That's okay
    // as long as we setMainWidget() and delete the old one.
    JobListingsUI *joblistingsui = new JobListingsUI(this, dbManager, sidebarjoblistwidget);
    setMainWidget(joblistingsui);

    if (previousButton != nullptr) {
        previousButton->setEnabled(true);
        previousButton = nullptr;
    }
}

//---------------------------------------------------------------------------------
// PROCESS EXECUTION
//---------------------------------------------------------------------------------
void MainWindow::on_ExecuteBtn_clicked()
{
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
            // Process finished
            isProcessRunning = false;
            setExecutionStateUI();
        });
        connect(worker, &ProcessWorker::processError, this, [this](const QString& errorMessage) {
            qDebug() << errorMessage;
            // Process error
            isProcessRunning = false;
            setExecutionStateUI();
        });

        connect(worker, &ProcessWorker::processFinished, thread, &QThread::quit);
        connect(worker, &ProcessWorker::processFinished, worker, &ProcessWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        // Reset worker/thread pointers after finishing
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
            QMetaObject::invokeMethod(worker, "stop", Qt::QueuedConnection);
        }
        // The state will be reset once the process finishes
    }
}

//---------------------------------------------------------------------------------
// MISC: UI Changes
//---------------------------------------------------------------------------------
void MainWindow::setExecutionStateUI()
{
    if (isProcessRunning) {
        ui->ExecuteBtn->setText("Stop Execution");
        ui->applyModeCheckbox->setDisabled(true);
    } else {
        ui->ExecuteBtn->setText("Execute");
        ui->applyModeCheckbox->setDisabled(false);
    }
}
