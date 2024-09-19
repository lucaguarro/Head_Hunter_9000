#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize the database manager with the path to the SQLite file
    dbManager = new DatabaseManager("/home/luca/Documents/Projects/Head_Hunter_9000/example.db");

    if (dbManager->connectToDatabase()) {
        loadQuestions();
    }
}

MainWindow::~MainWindow() {
    delete ui;
    delete dbManager;
}

void MainWindow::loadQuestions() {
    QSqlQuery query = dbManager->fetchQuestions();  // Fetch questions using the DatabaseManager

    while (query.next()) {
        QString questionText = query.value("question").toString();
        QString questionType = query.value("type").toString();

        qDebug() << "Question:" << questionText << "Type:" << questionType;

        // Add each question to the panel
        addQuestionToPanel(questionText, questionType);
    }
}

void MainWindow::addQuestionToPanel(const QString &questionText, const QString &questionType) {
    QVBoxLayout *mainPanelLayout = qobject_cast<QVBoxLayout *>(ui->MainPanel->layout());

    // Add a label for the question text
    QLabel *questionLabel = new QLabel(questionText);
    mainPanelLayout->addWidget(questionLabel);

    // Depending on the question type, create the appropriate input widget
    if (questionType == "FREERESPONSE") {
        QLineEdit *freeResponseInput = new QLineEdit();
        mainPanelLayout->addWidget(freeResponseInput);
    } else if (questionType == "RADIOBUTTON") {
        QWidget *radioGroupWidget = new QWidget();
        QVBoxLayout *radioLayout = new QVBoxLayout(radioGroupWidget);
        QRadioButton *option1 = new QRadioButton("Option 1");
        QRadioButton *option2 = new QRadioButton("Option 2");

        radioLayout->addWidget(option1);
        radioLayout->addWidget(option2);

        mainPanelLayout->addWidget(radioGroupWidget);
    } else if (questionType == "DROPDOWN") {
        QComboBox *dropdown = new QComboBox();
        dropdown->addItem("Option 1");
        dropdown->addItem("Option 2");

        mainPanelLayout->addWidget(dropdown);
    }
}
