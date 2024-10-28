#ifndef ASKQUESTIONSUI_H
#define ASKQUESTIONSUI_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QHBoxLayout>
#include "databasemanager.h"

class AskQuestionsUI : public QWidget
{
    Q_OBJECT

public:

    // Provides access to the content layout (for adding questions)
    QVBoxLayout* getContentLayout() const;

    AskQuestionsUI(QWidget *parent, DatabaseManager *dbmanager);

    ~AskQuestionsUI();
private:
    QVBoxLayout *contentLayout;
    QWidget *scrollContent;
    QHBoxLayout *buttonLayout;

    // Member for managing the database connection
    DatabaseManager *dbManager;

    void createSaveButton(QVBoxLayout *mainAreaLayout);
    void addQuestionToPanel(const QString &questionText, const QString &questionType, int questionId, bool isMultiLine);
    void loadQuestions();
    void saveAnswers();
};

#endif // ASKQUESTIONSUI_H
