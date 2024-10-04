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

QList<QPair<QString, int>> DatabaseManager::fetchOptionsForQuestion(const QString &questionType, int questionId) {
    QList<QPair<QString, int>> options;  // List to store the options (text and ID)

    QSqlQuery query;
    if (questionType == "radio buttons"){
        query.prepare(
            "SELECT o.id, o.text "
            "FROM option o "
            "INNER JOIN optionsetoption oso ON o.id = oso.optionid "
            "INNER JOIN optionset os ON oso.optionsetid = os.id "
            "INNER JOIN radiobuttonquestion rbq ON os.id = rbq.optionsetid "
            "WHERE rbq.id = :questionId;"
        );
    }
    else if (questionType == "drop down"){
        query.prepare(
            "SELECT o.id, o.text "
            "FROM option o "
            "INNER JOIN optionsetoption oso ON o.id = oso.optionid "
            "INNER JOIN optionset os ON oso.optionsetid = os.id "
            "INNER JOIN dropdownquestion ddq ON os.id = ddq.optionsetid "
            "WHERE ddq.id = :questionId;"
            );
    }

    // Bind the parameter only once
    qDebug() << "Binding questionId:" << questionId;
    query.bindValue(":questionId", questionId);

    // Execute the query and fetch results
    if (query.exec()) {
        while (query.next()) {
            int optionId = query.value(0).toInt();
            QString optionText = query.value(1).toString();
            options.append(qMakePair(optionText, optionId));  // Append to the list
            qDebug() << "Fetched option:" << optionText << " with ID:" << optionId;
        }
    } else {
        qDebug() << "Error fetching options:" << query.lastError();
    }

    return options;
}
