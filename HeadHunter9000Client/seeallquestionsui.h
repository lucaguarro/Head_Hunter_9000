#ifndef SEEALLQUESTIONSUI_H
#define SEEALLQUESTIONSUI_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include "databasemanager.h"

class SeeAllQuestionsUI : public QWidget
{
    Q_OBJECT

public:
    explicit SeeAllQuestionsUI(QWidget *parent = nullptr, DatabaseManager *dbManager = nullptr);
    ~SeeAllQuestionsUI();

private slots:
    void loadQuestions(); // Load questions into the table
    void updateAnswers(); // Update answers in the database
    // void showEvent(QShowEvent *event);

private:
    QVBoxLayout* mainLayout;  // Main layout of the widget
    QTableWidget* tableWidget;  // Table to display questions
    QPushButton* updateButton;  // Update button to save answers
    DatabaseManager* dbManager;  // Database manager

    QLineEdit *searchBox;

    QWidget* createEditorWidget(const QString& questionType, int questionId, const QList<QPair<QString, int>>& options, const QString& currentAnswer);
    QString getCheckboxAnswersAsText(const QList<QCheckBox*>& checkboxes);
    QSet<int> modifiedQuestionIds;

    // Helper function to update data back to the database
    void saveDropdownAnswer(int questionId, int selectedOptionId);
    void saveRadioButtonAnswer(int questionId, int selectedOptionId);
    void saveCheckboxAnswer(int questionId, const QList<int>& selectedOptionIds);
    void filterQuestions(const QString &searchText);
    void markQuestionAsModified(int questionId);
};

#endif // SEEALLQUESTIONSUI_H
