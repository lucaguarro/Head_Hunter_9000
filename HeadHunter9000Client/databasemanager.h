#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>

class DatabaseManager {
public:
    DatabaseManager(const QString &databasePath);

    bool connectToDatabase();
    QList<QPair<QString, int>> fetchOptionsForQuestion(const QString &questionType, int questionId);

    // New methods for updating the answers
    bool updateFreeResponseAnswer(int questionId, const QString &answerText);
    bool updateRadioButtonAnswer(int questionId, int optionId);
    bool updateDropdownAnswer(int questionId, int optionId);
    bool updateCheckboxQuestion(int questionId, const QList<int> &selectedOptionIds);
    QString fetchAnswerForQuestion(int questionId, const QString &questionType);
    QSqlQuery fetchQuestions(bool excludeAnswered);
private:
    QString databasePath;
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
