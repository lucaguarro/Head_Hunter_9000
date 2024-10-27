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
    bool excludeAnswered = true;
    QSqlQuery query = dbManager->fetchQuestions(excludeAnswered);  // Fetch questions using the DatabaseManager

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
    // Create a container widget for the question label and input
    QWidget *questionContainer = new QWidget(this);  // Assign 'this' as the parent
    QVBoxLayout *questionLayout = new QVBoxLayout(questionContainer);

    // Add a label for the question text
    QLabel *questionLabel = new QLabel(questionText, this);  // Assign 'this' as the parent
    questionLabel->setWordWrap(true);
    questionLayout->addWidget(questionLabel);

    QList<QPair<QString, int>> options;  // Declare options before the if statement

    if (questionType != "free response") {
        options = dbManager->fetchOptionsForQuestion(questionType, questionId);
    }

    // Depending on the question type, create the appropriate input widget
    if (questionType == "free response") {
        if (isMultiLine) {
            QTextEdit *freeResponseInput = new QTextEdit(this);
            freeResponseInput->setProperty("questionId", questionId);
            freeResponseInput->setProperty("questionType", "free response");
            questionLayout->addWidget(freeResponseInput);
        } else {
            QLineEdit *freeResponseInput = new QLineEdit(this);
            freeResponseInput->setProperty("questionId", questionId);
            freeResponseInput->setProperty("questionType", "free response");
            questionLayout->addWidget(freeResponseInput);
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

        questionLayout->addWidget(radioGroupWidget);
    } else if (questionType == "drop down") {
        QComboBox *dropdown = new QComboBox(this);  // Assign 'this' as the parent
        dropdown->addItem("Select an option", -1);

        // Populate dropdown with options from the database
        for (const auto &option : options) {
            dropdown->addItem(option.first, option.second);  // Use option text, and store option ID
        }

        dropdown->setProperty("questionId", questionId);
        dropdown->setProperty("questionType", "drop down");
        questionLayout->addWidget(dropdown);
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

        questionLayout->addWidget(checkboxGroupWidget);
    }

    // Add the entire question container to the content layout
    this->contentLayout->addWidget(questionContainer);
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
        QWidget *questionContainer = contentLayout->itemAt(i)->widget();

        if (!questionContainer) {
            continue;  // If it's not a widget, skip
        }

        QString questionType = questionContainer->property("questionType").toString();
        bool answered = false;  // Track if this question was answered

        // Iterate through children of questionContainer to find the input widget
        for (QWidget *child : questionContainer->findChildren<QWidget*>()) {
            QString childQuestionType = child->property("questionType").toString();

            // Handle free response questions (QLineEdit)
            if (childQuestionType == "free response") {
                QLineEdit *freeResponseInput = qobject_cast<QLineEdit*>(child);
                if (freeResponseInput) {
                    int questionId = freeResponseInput->property("questionId").toInt();
                    QString answerText = freeResponseInput->text().trimmed();  // Trim whitespace

                    if (!answerText.isEmpty()) {
                        // Only update if there's a non-empty answer
                        if (dbManager->updateFreeResponseAnswer(questionId, answerText)) {
                            answered = true;
                        } else {
                            qDebug() << "Failed to update free response answer!";
                        }
                    }
                }

                // Handle radio button questions (QRadioButton)
            } else if (childQuestionType == "radio buttons") {
                QWidget *radioGroupWidget = qobject_cast<QWidget*>(child);
                if (radioGroupWidget) {
                    int questionId = radioGroupWidget->property("questionId").toInt();
                    QVBoxLayout *radioLayout = qobject_cast<QVBoxLayout*>(radioGroupWidget->layout());

                    if (radioLayout) {
                        for (int j = 0; j < radioLayout->count(); ++j) {
                            QRadioButton *radioButton = qobject_cast<QRadioButton*>(radioLayout->itemAt(j)->widget());
                            if (radioButton && radioButton->isChecked()) {
                                int optionId = radioButton->property("optionId").toInt();
                                if (dbManager->updateRadioButtonAnswer(questionId, optionId)) {
                                    answered = true;
                                } else {
                                    qDebug() << "Failed to update radio button answer!";
                                }
                                break;  // Stop after finding the selected radio button
                            }
                        }
                    }
                }

                // Handle dropdown questions (QComboBox)
            } else if (childQuestionType == "drop down") {
                QComboBox *dropdown = qobject_cast<QComboBox*>(child);
                if (dropdown) {
                    int questionId = dropdown->property("questionId").toInt();
                    int selectedOptionId = dropdown->currentData().toInt();  // Get the optionId for the selected item

                    if (selectedOptionId != -1) {  // Ensure an option is selected
                        if (dbManager->updateDropdownAnswer(questionId, selectedOptionId)) {
                            answered = true;
                        } else {
                            qDebug() << "Failed to update dropdown answer!";
                        }
                    }
                }

                // Handle checkbox questions (QCheckBox)
            } else if (childQuestionType == "checkbox") {
                QWidget *checkboxGroupWidget = qobject_cast<QWidget*>(child);
                if (checkboxGroupWidget) {
                    int questionId = checkboxGroupWidget->property("questionId").toInt();
                    QVBoxLayout *checkboxLayout = qobject_cast<QVBoxLayout*>(checkboxGroupWidget->layout());
                    QList<int> selectedOptionIds;

                    // Iterate through all checkboxes and collect the selected ones
                    if (checkboxLayout) {
                        for (int j = 0; j < checkboxLayout->count(); ++j) {
                            QCheckBox *checkBox = qobject_cast<QCheckBox*>(checkboxLayout->itemAt(j)->widget());
                            if (checkBox && checkBox->isChecked()) {
                                int optionId = checkBox->property("optionId").toInt();
                                selectedOptionIds.append(optionId);
                            }
                        }

                        // Only update if there are selected checkboxes
                        if (!selectedOptionIds.isEmpty()) {
                            if (dbManager->updateCheckboxQuestion(questionId, selectedOptionIds)) {
                                answered = true;
                            } else {
                                qDebug() << "Failed to update checkbox question!";
                            }
                        }
                    }
                }
            }
        }

        // If the question was answered, remove the entire questionContainer (including label and input)
        if (answered) {
            contentLayout->removeWidget(questionContainer);  // Remove the container from the layout
            questionContainer->deleteLater();  // Schedule the container for deletion
            --i;  // Adjust index because the layout count has changed
        }
    }
}




