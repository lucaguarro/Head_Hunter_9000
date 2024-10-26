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
#include <QCheckBox>
#include <QTextEdit>

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
        bool isMultiLine = query.value("ismultiline").toBool();  // Retrieve isMultiLine

        qDebug() << "Question:" << questionText << "Type:" << questionType << "isMultiLine:" << isMultiLine;

        // Add each question to the panel, including isMultiLine
        addQuestionToPanel(questionText, questionType, questionId, isMultiLine);
    }
}

void AskQuestionsUI::addQuestionToPanel(const QString &questionText, const QString &questionType, int questionId, bool isMultiLine) {
    // Add a label for the question text
    QLabel *questionLabel = new QLabel(questionText, this);  // Assign 'this' as the parent
    questionLabel->setWordWrap(true);
    this->contentLayout->addWidget(questionLabel);

    QList<QPair<QString, int>> options;  // Declare options before the if statement

    if (questionType != "free response") {
        options = dbManager->fetchOptionsForQuestion(questionType, questionId);
    }

    // Depending on the question type, create the appropriate input widget
    if (questionType == "free response") {
        if (isMultiLine) {
            // Create a QTextEdit for multi-line input
            QTextEdit *freeResponseInput = new QTextEdit(this);
            freeResponseInput->setProperty("questionId", questionId);
            freeResponseInput->setProperty("questionType", "free response");
            this->contentLayout->addWidget(freeResponseInput);
        } else {
            // Create a QLineEdit for single-line input
            QLineEdit *freeResponseInput = new QLineEdit(this);
            freeResponseInput->setProperty("questionId", questionId);
            freeResponseInput->setProperty("questionType", "free response");
            this->contentLayout->addWidget(freeResponseInput);
        }
    } else if (questionType == "radio buttons") {
        QWidget *radioGroupWidget = new QWidget(this);  // Assign 'this' as the parent
        QVBoxLayout *radioLayout = new QVBoxLayout(radioGroupWidget);

        radioGroupWidget->setProperty("questionId", questionId);
        radioGroupWidget->setProperty("questionType", "radio buttons");

        // Populate radio buttons with options from the database
        for (const auto &option : options) {
            QRadioButton *radioButton = new QRadioButton(option.first, this);  // Assign 'this' as the parent
            radioButton->setProperty("optionId", option.second);  // Store option ID as property
            radioLayout->addWidget(radioButton);
        }

        this->contentLayout->addWidget(radioGroupWidget);
    } else if (questionType == "drop down") {
        QComboBox *dropdown = new QComboBox(this);  // Assign 'this' as the parent
        dropdown->addItem("Select an option", -1);

        // Populate dropdown with options from the database
        for (const auto &option : options) {
            dropdown->addItem(option.first, option.second);  // Use option text, and store option ID
        }

        dropdown->setProperty("questionId", questionId);
        dropdown->setProperty("questionType", "drop down");
        this->contentLayout->addWidget(dropdown);
    } else if (questionType == "checkbox") {
        QWidget *checkboxGroupWidget = new QWidget(this);  // Assign 'this' as the parent
        QVBoxLayout *checkboxLayout = new QVBoxLayout(checkboxGroupWidget);

        checkboxGroupWidget->setProperty("questionId", questionId);
        checkboxGroupWidget->setProperty("questionType", "checkbox");

        // Populate checkboxes with options from the database
        for (const auto &option : options) {
            QCheckBox *checkBox = new QCheckBox(option.first, this);  // Assign 'this' as the parent
            checkBox->setProperty("optionId", option.second);  // Store option ID as property
            checkboxLayout->addWidget(checkBox);
        }

        this->contentLayout->addWidget(checkboxGroupWidget);
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

    // Iterate over all widgets in the contentLayout
    for (int i = 0; i < contentLayout->count(); ++i) {
        QWidget *widget = contentLayout->itemAt(i)->widget();

        if (!widget) {
            continue;  // If it's not a widget, skip
        }

        QString questionType = widget->property("questionType").toString();

        // Handle free response questions (QLineEdit)
        if (questionType == "free response") {
            QLineEdit *freeResponseInput = qobject_cast<QLineEdit*>(widget);
            if (freeResponseInput) {
                int questionId = freeResponseInput->property("questionId").toInt();
                QString answerText = freeResponseInput->text();

                if (!dbManager->updateFreeResponseAnswer(questionId, answerText)) {
                    qDebug() << "Failed to update free response answer!";
                }
            }

            // Handle radio button questions (inside QWidget)
        } else if (questionType == "radio buttons") {
            QWidget *radioGroupWidget = qobject_cast<QWidget*>(widget);
            if (radioGroupWidget) {
                int questionId = radioGroupWidget->property("questionId").toInt();

                QVBoxLayout *radioLayout = qobject_cast<QVBoxLayout*>(radioGroupWidget->layout());
                if (radioLayout) {
                    for (int j = 0; j < radioLayout->count(); ++j) {
                        QRadioButton *radioButton = qobject_cast<QRadioButton*>(radioLayout->itemAt(j)->widget());
                        if (radioButton && radioButton->isChecked()) {
                            int optionId = radioButton->property("optionId").toInt();

                            if (!dbManager->updateRadioButtonAnswer(questionId, optionId)) {
                                qDebug() << "Failed to update radio button answer!";
                            }
                            break;  // Stop after finding the selected radio button
                        }
                    }
                }
            }

            // Handle dropdown questions (QComboBox)
        } else if (questionType == "drop down") {
            QComboBox *dropdown = qobject_cast<QComboBox*>(widget);
            if (dropdown) {
                int questionId = dropdown->property("questionId").toInt();
                int selectedOptionId = dropdown->currentData().toInt();  // Get the optionId for the selected item

                if (!dbManager->updateDropdownAnswer(questionId, selectedOptionId)) {
                    qDebug() << "Failed to update dropdown answer!";
                }
            }

            // Handle checkbox questions
        } else if (questionType == "checkbox") {
            QWidget *checkboxGroupWidget = qobject_cast<QWidget*>(widget);
            if (checkboxGroupWidget) {
                int questionId = checkboxGroupWidget->property("questionId").toInt();

                QVBoxLayout *checkboxLayout = qobject_cast<QVBoxLayout*>(checkboxGroupWidget->layout());
                if (checkboxLayout) {
                    QList<int> selectedOptionIds;

                    // Iterate through all checkboxes and collect the selected ones
                    for (int j = 0; j < checkboxLayout->count(); ++j) {
                        QCheckBox *checkBox = qobject_cast<QCheckBox*>(checkboxLayout->itemAt(j)->widget());
                        if (checkBox && checkBox->isChecked()) {
                            int optionId = checkBox->property("optionId").toInt();
                            selectedOptionIds.append(optionId);
                        }
                    }

                    // Update the checkbox question with the selected options
                    if (!dbManager->updateCheckboxQuestion(questionId, selectedOptionIds)) {
                        qDebug() << "Failed to update checkbox question!";
                    }
                }
            }
        }
    }
}



