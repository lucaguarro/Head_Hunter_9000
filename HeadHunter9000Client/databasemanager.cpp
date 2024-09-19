#include "databasemanager.h"
#include <QDebug>
#include <QSqlError>

DatabaseManager::DatabaseManager(const QString &databasePath)
    : databasePath(databasePath) {}

bool DatabaseManager::connectToDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);

    if (!db.open()) {
        qDebug() << "Error: Could not connect to database!" << db.lastError().text();
        return false;
    }

    qDebug() << "Database connection successful!";
    return true;
}

QSqlQuery DatabaseManager::fetchQuestions() {
    QSqlQuery query;
    query.prepare("SELECT * FROM question");

    if (!query.exec()) {
        qDebug() << "Error fetching questions:" << query.lastError();
    }

    return query;  // Return the query object to be processed elsewhere
}
