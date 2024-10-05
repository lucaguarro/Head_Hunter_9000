#include "askquestionsui.h"
#include <QScrollArea>
#include <QPushButton>
#include <QDebug>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QComboBox>

AskQuestionsUI::AskQuestionsUI(QWidget *parent, DatabaseManager *dbManager)
    : QWidget(parent)
    , dbManager(dbManager)
{
    // Step 1: Create a layout for AskQuestionsUI
    QVBoxLayout* mainLayout = new QVBoxLayout(this);  // Set layout on 'this', which is AskQuestionsUI

    // Step 2: Create the QScrollArea
    QScrollArea* scrollArea = new QScrollArea(this);  // 'this' as parent ensures Qt manages it
    scrollContent = new QWidget(scrollArea);          // scrollContent's parent is scrollArea
    contentLayout = new QVBoxLayout(scrollContent);   // Set a layout for the scroll content

    // Step 3: Add the scroll area to the main layout
    scrollContent->setLayout(contentLayout);
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);

    // Add the scroll area to the main layout of AskQuestionsUI
    mainLayout->addWidget(scrollArea);
    this->setLayout(mainLayout);

    // Step 4: Create and add the Save button
    createSaveButton(mainLayout);

    // Load questions from the database
    if (dbManager->connectToDatabase()) {
        loadQuestions();
    }
}

// Destructor: No need to manually delete child widgets, Qt will handle it
AskQuestionsUI::~AskQuestionsUI() {
    qDebug() << "AskQuestionsUI has been destroyed, and all widgets are removed.";
}

// Function to return the content layout for adding questions dynamically
QVBoxLayout* AskQuestionsUI::getContentLayout() const {
    return contentLayout;
}

void AskQuestionsUI::loadQuestions() {
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

void AskQuestionsUI::addQuestionToPanel(const QString &questionText, const QString &questionType, int questionId) {
    // Add a label for the question text
    QLabel *questionLabel = new QLabel(questionText, this);  // Assign 'this' as the parent
    this->contentLayout->addWidget(questionLabel);

    QList<QPair<QString, int>> options;  // Declare options before the if statement

    if (questionType != "free response") {
        options = dbManager->fetchOptionsForQuestion(questionType, questionId);
    }

    // Depending on the question type, create the appropriate input widget
    if (questionType == "free response") {
        QLineEdit *freeResponseInput = new QLineEdit(this);  // Assign 'this' as the parent
        freeResponseInput->setProperty("questionId", questionId);
        this->contentLayout->addWidget(freeResponseInput);
    } else if (questionType == "radio buttons") {
        QWidget *radioGroupWidget = new QWidget(this);  // Assign 'this' as the parent
        QVBoxLayout *radioLayout = new QVBoxLayout(radioGroupWidget);

        radioGroupWidget->setProperty("questionId", questionId);

        // Populate radio buttons with options from the database
        for (const auto &option : options) {
            QRadioButton *radioButton = new QRadioButton(option.first, this);  // Assign 'this' as the parent
            radioButton->setProperty("optionId", option.second);  // Store option ID as property
            radioLayout->addWidget(radioButton);
        }

        radioGroupWidget->setProperty("questionId", questionId);
        this->contentLayout->addWidget(radioGroupWidget);
    } else if (questionType == "drop down") {
        QComboBox *dropdown = new QComboBox(this);  // Assign 'this' as the parent

        // Populate dropdown with options from the database
        for (const auto &option : options) {
            dropdown->addItem(option.first, option.second);  // Use option text, and store option ID
        }

        dropdown->setProperty("questionId", questionId);
        this->contentLayout->addWidget(dropdown);
    }
}

// Function to create the Save button and add it to the mainAreaLayout
void AskQuestionsUI::createSaveButton(QVBoxLayout *mainAreaLayout) {
    QPushButton *saveButton = new QPushButton("Save Answers", this);  // 'this' as parent ensures cleanup
    saveButton->setFixedHeight(26);  // Set a fixed height for the button
    saveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);  // Set fixed size policy

    // Connect the Save button's signal to your save logic
    connect(saveButton, &QPushButton::clicked, this, &AskQuestionsUI::saveAnswers);

    // Create a horizontal layout for the Save button and the spacer
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // Add a horizontal spacer to push the Save button to the right
    buttonLayout->addStretch();  // This will push the button to the right

    // Add the Save button to the horizontal layout
    buttonLayout->addWidget(saveButton);

    // Step 4: Add the horizontal layout (with the spacer and Save button) to the main area layout
    mainAreaLayout->addLayout(buttonLayout);
}

void AskQuestionsUI::saveAnswers() {
    qDebug() << "Save button clicked!";
    // Add your save logic here
}

