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
    if (questionType == "radio buttons") {
        query.prepare(
            "SELECT o.id, o.text "
            "FROM option o "
            "INNER JOIN optionsetoption oso ON o.id = oso.optionid "
            "INNER JOIN optionset os ON oso.optionsetid = os.id "
            "INNER JOIN radiobuttonquestion rbq ON os.id = rbq.optionsetid "
            "WHERE rbq.id = :questionId;"
            );
    } else if (questionType == "drop down") {
        query.prepare(
            "SELECT o.id, o.text "
            "FROM option o "
            "INNER JOIN optionsetoption oso ON o.id = oso.optionid "
            "INNER JOIN optionset os ON oso.optionsetid = os.id "
            "INNER JOIN dropdownquestion ddq ON os.id = ddq.optionsetid "
            "WHERE ddq.id = :questionId;"
            );
    } else if (questionType == "checkbox") {
        query.prepare(
            "SELECT o.id, o.text "
            "FROM option o "
            "INNER JOIN optionsetoption oso ON o.id = oso.optionid "
            "INNER JOIN optionset os ON oso.optionsetid = os.id "
            "INNER JOIN checkboxquestion ddq ON os.id = ddq.optionsetid "
            "WHERE ddq.id = :questionId;"
            );
    }


    query.bindValue(":questionId", questionId);

    if (query.exec()) {
        while (query.next()) {
            int optionId = query.value(0).toInt();
            QString optionText = query.value(1).toString();
            options.append(qMakePair(optionText, optionId));  // Append to the list
        }
    } else {
        qDebug() << "Error fetching options:" << query.lastError();
    }

    return options;
}

// New Methods to Update Answers
bool DatabaseManager::updateFreeResponseAnswer(int questionId, const QString &answerText) {
    QSqlQuery query;
    query.prepare("UPDATE freeresponsequestion SET answer = :widgettext WHERE id = :questionid");
    query.bindValue(":widgettext", answerText);
    query.bindValue(":questionid", questionId);

    if (!query.exec()) {
        qDebug() << "Error updating free response question:" << query.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::updateRadioButtonAnswer(int questionId, int optionId) {
    QSqlQuery query;
    query.prepare("UPDATE radiobuttonquestion SET answerasoptionid = :optionid WHERE id = :questionid");
    query.bindValue(":optionid", optionId);
    query.bindValue(":questionid", questionId);

    if (!query.exec()) {
        qDebug() << "Error updating radio button question:" << query.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::updateDropdownAnswer(int questionId, int optionId) {
    QSqlQuery query;
    query.prepare("UPDATE dropdownquestion SET answerasoptionid = :optionid WHERE id = :questionid");
    query.bindValue(":optionid", optionId);
    query.bindValue(":questionid", questionId);

    if (!query.exec()) {
        qDebug() << "Error updating dropdown question:" << query.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::updateCheckboxQuestion(int questionId, const QList<int> &selectedOptionIds) {
    QSqlQuery query;

    // Start by removing any answers that are not in the selectedOptionIds
    query.prepare("DELETE FROM checkboxanswers WHERE checkboxquestionid = :questionId AND answerasoptionid NOT IN (:selectedOptionIds)");
    query.bindValue(":questionId", questionId);
    query.bindValue(":selectedOptionIds", QVariant::fromValue(selectedOptionIds));  // Binds the selectedOptionIds list
    if (!query.exec()) {
        qDebug() << "Failed to delete unselected checkbox answers:" << query.lastError();
        return false;
    }

    // Now ensure that all selected options are in the database
    for (int optionId : selectedOptionIds) {
        query.prepare("INSERT OR IGNORE INTO checkboxanswers (checkboxquestionid, answerasoptionid) VALUES (:questionId, :optionId)");
        query.bindValue(":questionId", questionId);
        query.bindValue(":optionId", optionId);
        if (!query.exec()) {
            qDebug() << "Failed to insert selected checkbox answer:" << query.lastError();
            return false;
        }
    }

    return true;
}
