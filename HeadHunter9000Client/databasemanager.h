#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSettings>
#include <QList>
#include "job.h"

class DatabaseManager {
public:
    explicit DatabaseManager(QSettings* settings);


    bool connectToDatabase();
    QList<QPair<QString, int>> fetchOptionsForQuestion(const QString &questionType, int questionId);

    // New methods for updating the answers
    bool updateFreeResponseAnswer(int questionId, const QString &answerText);
    bool updateRadioButtonAnswer(int questionId, int optionId);
    bool updateDropdownAnswer(int questionId, int optionId);
    bool updateCheckboxQuestion(int questionId, const QList<int> &selectedOptionIds);
    QString fetchAnswerForQuestion(int questionId, const QString &questionType);
    QSqlQuery fetchQuestions(bool excludeAnswered);
    void setDatabasePath();

    QList<Job> getJobs();
    void updateJobPreferenceScore(int jobId, int newScore);
private:
    QString databasePath;
    QSqlDatabase db;
    QSettings* settings;
};

#endif // DATABASEMANAGER_H
