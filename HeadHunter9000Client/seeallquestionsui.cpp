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

    // Step 4: Load questions from the database
    if (dbManager->connectToDatabase()) {
        loadQuestions();
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
            connect(checkBox, &QCheckBox::stateChanged, [comboBox, checkboxes, this]() {
                comboBox->setCurrentText(getCheckboxAnswersAsText(checkboxes));
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

void SeeAllQuestionsUI::updateAnswers()
{
    qDebug() << "test";
    // for (int i = 0; i < tableWidget->rowCount(); ++i) {
    //     QWidget* widget = tableWidget->cellWidget(i, 3);  // Get the answer widget in column 4

    //     if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
    //         int questionId = lineEdit->property("questionId").toInt();
    //         QString answerText = lineEdit->text();
    //         dbManager->updateFreeResponseAnswer(questionId, answerText);
    //     }
    //     else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
    //         int questionId = comboBox->property("questionId").toInt();
    //         if (comboBox->property("checkboxes").isValid()) {
    //             // Checkbox question
    //             QList<QCheckBox*> checkboxes = comboBox->property("checkboxes").value<QList<QCheckBox*>>();
    //             QList<int> selectedOptionIds;
    //             for (QCheckBox* checkBox : checkboxes) {
    //                 if (checkBox->isChecked()) {
    //                     selectedOptionIds.append(checkBox->property("optionId").toInt());
    //                 }
    //             }
    //             dbManager->updateCheckboxQuestion(questionId, selectedOptionIds);
    //         } else {
    //             // Drop down or radio button
    //             int selectedOptionId = comboBox->currentData().toInt();
    //             dbManager->updateDropdownOrRadioButtonAnswer(questionId, selectedOptionId);
    //         }
    //     }
    // }
}
