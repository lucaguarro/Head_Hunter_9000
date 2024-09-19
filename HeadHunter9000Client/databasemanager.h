#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>

class DatabaseManager {
public:
    DatabaseManager(const QString &databasePath);
    bool connectToDatabase();
    QSqlQuery fetchQuestions();

private:
    QSqlDatabase db;
    QString databasePath;
};

#endif // DATABASEMANAGER_H
