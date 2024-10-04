#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QComboBox>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize the database manager with the path to the SQLite file
    dbManager = new DatabaseManager("/home/luca/Documents/Projects/Head_Hunter_9000/example.db");

    // Create a QScrollArea and set MainPanel as its widget
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(ui->MainPanel);  // Set MainPanel as the scrollable content
    scrollArea->setWidgetResizable(true);  // Allow the scroll area to resize with content

    // Replace the MainPanel in the central widget layout with the scroll area
    ui->centralwidget->layout()->addWidget(scrollArea);

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
        int questionId = query.value("id").toInt();

        qDebug() << "Question:" << questionText << "Type:" << questionType;

        // Add each question to the panel
        addQuestionToPanel(questionText, questionType, questionId);
    }
}

void MainWindow::addQuestionToPanel(const QString &questionText, const QString &questionType, int questionId) {
    QVBoxLayout *mainPanelLayout = qobject_cast<QVBoxLayout *>(ui->MainPanel->layout());

    // Add a label for the question text
    QLabel *questionLabel = new QLabel(questionText);
    mainPanelLayout->addWidget(questionLabel);

    QList<QPair<QString, int>> options;  // Declare options before the if statement

    if (questionType != "free response"){
        options = dbManager->fetchOptionsForQuestion(questionType, questionId);
    }

    // Depending on the question type, create the appropriate input widget
    if (questionType == "free response") {
        QLineEdit *freeResponseInput = new QLineEdit();
        freeResponseInput->setProperty("questionId", questionId);
        mainPanelLayout->addWidget(freeResponseInput);
    } else if (questionType == "radio buttons") {
        QWidget *radioGroupWidget = new QWidget();
        QVBoxLayout *radioLayout = new QVBoxLayout(radioGroupWidget);

        radioGroupWidget->setProperty("questionId", questionId);

        // Populate radio buttons with options from the database
        for (const auto &option : options) {
            QRadioButton *radioButton = new QRadioButton(option.first);  // Use option text
            radioButton->setProperty("optionId", option.second);  // Store option ID as property
            radioLayout->addWidget(radioButton);
        }

        mainPanelLayout->addWidget(radioGroupWidget);
    } else if (questionType == "drop down") {
        QComboBox *dropdown = new QComboBox();

        // Populate dropdown with options from the database
        for (const auto &option : options) {
            dropdown->addItem(option.first, option.second);  // Use option text, and store option ID
        }

        dropdown->setProperty("questionId", questionId);
        mainPanelLayout->addWidget(dropdown);
    }
}
