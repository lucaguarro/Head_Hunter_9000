#include "seeallquestionsui.h"
#include "checkablecombobox.h"
#include "qlineedit.h"
#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>
#include <QMenu>
#include <QVBoxLayout>
#include <QWidgetAction>

SeeAllQuestionsUI::SeeAllQuestionsUI(QWidget *parent, DatabaseManager *dbManager)
    : QWidget(parent), dbManager(dbManager)
{
    // Step 1: Create main layout and table
    mainLayout = new QVBoxLayout(this);

    // Create search box for filtering questions
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Search questions...");
    mainLayout->addWidget(searchBox);

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"Id", "Question Type", "Question", "Answer"});

    QHeaderView *header = tableWidget->horizontalHeader();

    // Allow the user to resize columns
    header->setSectionResizeMode(QHeaderView::Interactive);

    // Stretch the last section to fill remaining space
    header->setStretchLastSection(true);

    // Optional: Enable cascading section resizes
    header->setCascadingSectionResizes(true);

    // Set size policy to ensure table takes up all available space
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Disable horizontal scrollbar to prevent table from expanding beyond layout
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Optional: Adjust vertical scrollbar policy
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tableWidget->setSortingEnabled(true);  // Enable sorting by columns

    mainLayout->addWidget(tableWidget);

    // Step 3: Create update button
    updateButton = new QPushButton("Update Answers", this);
    connect(updateButton, &QPushButton::clicked, this, &SeeAllQuestionsUI::updateAnswers);
    mainLayout->addWidget(updateButton);

    // Step 4: Connect searchBox text change to filter function
    connect(searchBox, &QLineEdit::textChanged, this, &SeeAllQuestionsUI::filterQuestions);


    // Step 4: Load questions from the database
    if (dbManager->connectToDatabase()) {
        loadQuestions();
    }
}

void SeeAllQuestionsUI::filterQuestions(const QString &searchText)
{
    // Loop through all rows in the table
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // Get the question text from column 2
        QTableWidgetItem* questionItem = tableWidget->item(row, 2);
        if (questionItem) {
            QString questionText = questionItem->text();

            // Check if the question contains the search text (case-insensitive)
            bool match = questionText.contains(searchText, Qt::CaseInsensitive);

            // Show or hide the row based on whether the text matches
            tableWidget->setRowHidden(row, !match);
        }
    }
}

SeeAllQuestionsUI::~SeeAllQuestionsUI()
{
    qDebug() << "SeeAllQuestionsUI has been destroyed.";
}

void SeeAllQuestionsUI::loadQuestions()
{
    // Fetch all questions from the database regardless of whether they are answered or not
    bool excludeAnswered = false;
    QSqlQuery query = dbManager->fetchQuestions(excludeAnswered);

    int row = 0;
    while (query.next()) {
        int questionId = query.value("id").toInt();
        QString questionType = query.value("type").toString();
        QString questionText = query.value("question").toString();
        QString currentAnswer = dbManager->fetchAnswerForQuestion(questionId, questionType);

        // Add a new row to the table
        tableWidget->insertRow(row);

        // Set the Id, Question Type, and Question
        tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(questionId)));
        tableWidget->setItem(row, 1, new QTableWidgetItem(questionType));
        tableWidget->setItem(row, 2, new QTableWidgetItem(questionText));

        // Fetch options for non-free-response questions
        QList<QPair<QString, int>> options;
        if (questionType != "free response") {
            options = dbManager->fetchOptionsForQuestion(questionType, questionId);
        }

        // Create the appropriate editor widget for the answer column
        QWidget* editorWidget = createEditorWidget(questionType, questionId, options, currentAnswer);
        tableWidget->setCellWidget(row, 3, editorWidget);

        row++;
    }
}

QWidget* SeeAllQuestionsUI::createEditorWidget(const QString& questionType, int questionId, const QList<QPair<QString, int>>& options, const QString& currentAnswer)
{
    if (questionType == "free response") {
        QLineEdit* lineEdit = new QLineEdit(currentAnswer, this);
        lineEdit->setProperty("questionId", questionId);

        // Set the size policy to allow it to expand with the column
        lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        connect(lineEdit, &QLineEdit::textChanged, [this, questionId]() {
            markQuestionAsModified(questionId);
        });

        return lineEdit;
    }
    else if (questionType == "drop down" || questionType == "radio buttons") {
        QComboBox* comboBox = new QComboBox(this);

        // Add default option "Select an option"
        comboBox->addItem("Select an option", -1);  // Using -1 as a dummy value for the default option

        // Populate combo box with actual options
        for (const auto& option : options) {
            comboBox->addItem(option.first, option.second);
        }

        // If no current answer (empty string), set the default option as selected
        if (currentAnswer.isEmpty()) {
            comboBox->setCurrentText("Select an option");
        } else {
            comboBox->setCurrentText(currentAnswer);  // Set the current answer if it's available
        }

        // Set the size policy to allow it to expand with the column
        comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        comboBox->setProperty("questionId", questionId);

        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, questionId](int) {
            markQuestionAsModified(questionId);
        });

        return comboBox;
    }
    else if (questionType == "checkbox") {
        // Use the custom CheckableComboBox
        CheckableComboBox* comboBox = new CheckableComboBox(this);

        QList<QCheckBox*> checkboxes;
        QMenu* menu = new QMenu(comboBox);

        for (const auto& option : options) {
            // Create a QWidget to hold the checkbox with margins
            QWidget* checkboxWidget = new QWidget(menu);  // Set menu as parent
            QHBoxLayout* layout = new QHBoxLayout(checkboxWidget);
            layout->setContentsMargins(10, 0, 10, 0);  // Set left and right margins (adjust values as needed)
            layout->setSpacing(0);  // Optional: Set spacing between widgets to 0

            QCheckBox* checkBox = new QCheckBox(option.first, checkboxWidget);
            checkBox->setProperty("optionId", option.second);
            checkboxes.append(checkBox);

            layout->addWidget(checkBox);

            // Add the widget with checkbox to menu as an action
            QWidgetAction* action = new QWidgetAction(menu);
            action->setDefaultWidget(checkboxWidget);
            menu->addAction(action);

            // If the currentAnswer contains the option, mark it as checked
            if (currentAnswer.contains(option.first)) {
                checkBox->setChecked(true);
            }
        }

        // Set the initial text of the combo box
        comboBox->setCurrentText(getCheckboxAnswersAsText(checkboxes));

        // Connect checkbox state change to update the combo box display
        for (QCheckBox* checkBox : checkboxes) {
            connect(checkBox, &QCheckBox::stateChanged, [comboBox, checkboxes, this, questionId]() {
                comboBox->setCurrentText(getCheckboxAnswersAsText(checkboxes));
                markQuestionAsModified(questionId);
            });
        }

        comboBox->setMenu(menu);

        // Store checkboxes and the questionId in properties
        comboBox->setProperty("checkboxes", QVariant::fromValue(checkboxes));
        comboBox->setProperty("questionId", questionId);

        // Set the size policy to allow it to expand with the column
        comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        return comboBox;
    }

    return nullptr;
}

QString SeeAllQuestionsUI::getCheckboxAnswersAsText(const QList<QCheckBox*>& checkboxes)
{
    QStringList selectedOptions;
    for (QCheckBox* checkBox : checkboxes) {
        if (checkBox->isChecked()) {
            selectedOptions.append(checkBox->text());
        }
    }
    return selectedOptions.join("; ");
}

void SeeAllQuestionsUI::markQuestionAsModified(int questionId)
{
    modifiedQuestionIds.insert(questionId);
}

void SeeAllQuestionsUI::updateAnswers()
{
    // Iterate over all rows in the table
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QTableWidgetItem* idItem = tableWidget->item(row, 0); // Id column
        if (!idItem)
            continue;

        int questionId = idItem->text().toInt();

        // Check if this question was modified
        if (!modifiedQuestionIds.contains(questionId))
            continue;

        // Get the question type
        QTableWidgetItem* typeItem = tableWidget->item(row, 1);
        if (!typeItem)
            continue;

        QString questionType = typeItem->text();

        // Get the editor widget
        QWidget* editorWidget = tableWidget->cellWidget(row, 3); // Answer column
        if (!editorWidget)
            continue;

        if (questionType == "free response") {
            QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editorWidget);
            if (lineEdit) {
                QString answerText = lineEdit->text();
                // Update the database
                if (!dbManager->updateFreeResponseAnswer(questionId, answerText)) {
                    qDebug() << "Failed to update free response answer for question" << questionId;
                }
            }
        } else if (questionType == "drop down" || questionType == "radio buttons") {
            QComboBox* comboBox = qobject_cast<QComboBox*>(editorWidget);
            if (comboBox) {
                int optionId = comboBox->currentData().toInt(); // Get the option id
                if (optionId == -1) {
                    // No selection made; you may choose to handle this case
                    continue;
                }
                if (questionType == "radio buttons") {
                    if (!dbManager->updateRadioButtonAnswer(questionId, optionId)) {
                        qDebug() << "Failed to update radio button answer for question" << questionId;
                    }
                } else if (questionType == "drop down") {
                    if (!dbManager->updateDropdownAnswer(questionId, optionId)) {
                        qDebug() << "Failed to update dropdown answer for question" << questionId;
                    }
                }
            }
        } else if (questionType == "checkbox") {
            CheckableComboBox* comboBox = qobject_cast<CheckableComboBox*>(editorWidget);
            if (comboBox) {
                QMenu* menu = comboBox->m_menu;
                QList<int> selectedOptionIds;

                for (QAction* action : menu->actions()) {
                    QWidgetAction* widgetAction = qobject_cast<QWidgetAction*>(action);
                    if (widgetAction) {
                        QWidget* widget = widgetAction->defaultWidget();
                        QCheckBox* checkBox = widget->findChild<QCheckBox*>();
                        if (checkBox && checkBox->isChecked()) {
                            int optionId = checkBox->property("optionId").toInt();
                            selectedOptionIds.append(optionId);
                        }
                    }
                }

                if (!dbManager->updateCheckboxQuestion(questionId, selectedOptionIds)) {
                    qDebug() << "Failed to update checkbox answer for question" << questionId;
                }
            }
        }
    }

    // Clear the modifiedQuestionIds set after updating
    modifiedQuestionIds.clear();

    qDebug() << "Answers updated successfully";
}
