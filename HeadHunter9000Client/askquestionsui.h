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

    AskQuestionsUI(QVBoxLayout *mainAreaLayout, QWidget *parent, DatabaseManager *dbmanager);
private:
    QVBoxLayout *contentLayout;

    // Member for managing the database connection
    DatabaseManager *dbManager;

    void createSaveButton(QVBoxLayout *mainAreaLayout);
    void addQuestionToPanel(const QString &questionText, const QString &questionType, int questionId);
    void loadQuestions();
};

#endif // ASKQUESTIONSUI_H
