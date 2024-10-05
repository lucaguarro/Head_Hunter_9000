#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_AnswerQuestionsBtn_clicked();

private:
    Ui::MainWindow *ui;

    // Member for managing the database connection
    DatabaseManager *dbManager;

    // Function to dynamically add question widgets based on the question type
    void addQuestionToPanel(const QString &questionText, const QString &questionType, int questionId);

    // Function to load and display questions from the database
    void loadQuestions();
    void setupScrollAreaAndSaveButton();
    void saveData();
};

#endif // MAINWINDOW_H
