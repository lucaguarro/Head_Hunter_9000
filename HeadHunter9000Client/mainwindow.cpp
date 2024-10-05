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

    // SidebarMenu is at index 0, scrollArea is at index 1 in the splitter
    ui->splitter->setStretchFactor(0, 0);  // SidebarMenu gets no stretch (fixed size)
    ui->splitter->setStretchFactor(1, 1);  // scrollArea stretches to fill the remaining space
    ui->splitter->setSizes({200,1});

    QVBoxLayout *scrollAreaLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);  // Create and assign layout
    ui->scrollAreaWidgetContents->setLayout(scrollAreaLayout);
    ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
    ui->scrollArea->setWidgetResizable(true);
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
    QVBoxLayout *scrollAreaLayout = qobject_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());

    // Add a label for the question text
    QLabel *questionLabel = new QLabel(questionText);
    scrollAreaLayout->addWidget(questionLabel);

    QList<QPair<QString, int>> options;  // Declare options before the if statement

    if (questionType != "free response"){
        options = dbManager->fetchOptionsForQuestion(questionType, questionId);
    }

    // Depending on the question type, create the appropriate input widget
    if (questionType == "free response") {
        QLineEdit *freeResponseInput = new QLineEdit();
        freeResponseInput->setProperty("questionId", questionId);
        scrollAreaLayout->addWidget(freeResponseInput);
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

        scrollAreaLayout->addWidget(radioGroupWidget);
    } else if (questionType == "drop down") {
        QComboBox *dropdown = new QComboBox();

        // Populate dropdown with options from the database
        for (const auto &option : options) {
            dropdown->addItem(option.first, option.second);  // Use option text, and store option ID
        }

        dropdown->setProperty("questionId", questionId);
        scrollAreaLayout->addWidget(dropdown);
    }
}

void MainWindow::setupScrollAreaAndSaveButton() {
    // Create a vertical layout
    QVBoxLayout *verticalLayout = qobject_cast<QVBoxLayout *>(ui->scrollAreaContainer->layout());

    // Ensure the scrollArea is already created and set up
    if (!ui->scrollArea) {
        qDebug() << "scrollArea is null!";
        return;
    }

    // Add the scrollArea to the layout
    // verticalLayout->addWidget(ui->scrollArea);

    // Add a vertical stretch to push the Save button to the bottom
    // verticalLayout->addStretch(1);  // 1 is the stretch factor

    // Create the Save button
    QPushButton *saveButton = new QPushButton("Save");
    saveButton->setFixedHeight(40);  // Set a fixed height for the button (optional)
    saveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);  // Optional: control the size policy

    // Connect the Save button's signal to the save function
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveData);

    // Add the Save button to the layout at the bottom
    verticalLayout->addWidget(saveButton);

    // Set the layout to the parent widget (e.g., centralwidget)
    // ui->centralwidget->setLayout(verticalLayout);
}

// Function to handle the Save button action
void MainWindow::saveData() {
    qDebug() << "Save button clicked!";
    // Add your save logic here
}



void MainWindow::on_AnswerQuestionsBtn_clicked()
{
    setupScrollAreaAndSaveButton();
    if (dbManager->connectToDatabase()) {
        loadQuestions();
    }
}

