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
    , previousButton(nullptr)
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
            cleanUpAnswerQuestionsPage();
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
    // Create a vertical layout for the scroll area and save button
    QVBoxLayout *verticalLayout = qobject_cast<QVBoxLayout *>(ui->scrollAreaContainer->layout());

    // Ensure the scrollArea is already created and set up
    if (!ui->scrollArea) {
        qDebug() << "scrollArea is null!";
        return;
    }

    // Create the Save button
    QPushButton *saveButton = new QPushButton("Save Answers");
    saveButton->setFixedHeight(26);  // Set a fixed height for the button (optional)
    saveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);  // Set fixed size policy

    // Connect the Save button's signal to the save function
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveData);

    // Create a horizontal layout for the Save button and the spacer
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // Add a horizontal spacer to push the Save button to the right
    buttonLayout->addStretch();  // This will push the button to the right

    // Add the Save button to the horizontal layout
    buttonLayout->addWidget(saveButton);

    // Add the horizontal layout (with the spacer and Save button) to the vertical layout
    verticalLayout->addLayout(buttonLayout);
}

// Function to handle the Save button action
void MainWindow::saveData() {
    qDebug() << "Save button clicked!";
    // Add your save logic here
}



void MainWindow::on_AnswerQuestionsBtn_clicked()
{
    ui->AnswerQuestionsBtn->setEnabled(false);
    setupScrollAreaAndSaveButton();
    if (dbManager->connectToDatabase()) {
        loadQuestions();
    }
}

