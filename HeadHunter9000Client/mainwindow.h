#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
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

    QPushButton* previousButton;

    // Member for managing the database connection
    DatabaseManager *dbManager;


    // Function to load and display questions from the database
    void saveData();
    void onSidebarButtonClicked(QPushButton *clickedButton, const QList<QPushButton *> &buttons);
    void cleanUpAnswerQuestionsPage();
    void cleanUpSeeAllQuestionsPage();
    void cleanUpScraperConfigPage();
    void cleanUpJobSearchCriteriaPage();
    QVBoxLayout *setupScrollAreaAndSaveButton();
    void loadQuestions(QVBoxLayout *contentLayout);
    void addQuestionToPanel(QVBoxLayout *contentLayout, const QString &questionText, const QString &questionType, int questionId);
};

#endif // MAINWINDOW_H
