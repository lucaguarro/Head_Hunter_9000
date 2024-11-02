#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include "askquestionsui.h"
#include "databasemanager.h"
#include "scraperconfigurationui.h"
#include "seeallquestionsui.h"
#include "processworker.h"

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
    void on_SeeAllQuestionsBtn_clicked();
    void on_ScraperConfigBtn_clicked();
    void on_ExecuteBtn_clicked();

private:
    Ui::MainWindow *ui;
    AskQuestionsUI* askquestionsui;
    SeeAllQuestionsUI* seeallquestionsui;
    ScraperConfigurationUI* scraperconfigurationui;
    QSettings* settings;

    QPushButton* previousButton;

    // Member for managing the database connection
    DatabaseManager *dbManager;

    bool isProcessRunning = false;
    ProcessWorker* worker = nullptr;
    QThread* thread = nullptr;

    // Function to load and display questions from the database
    void saveData();
    void onSidebarButtonClicked(QPushButton *clickedButton, const QList<QPushButton *> &buttons);
    void cleanUpScraperConfigPage();
    void cleanUpJobSearchCriteriaPage();
    QVBoxLayout *setupScrollAreaAndSaveButton();
    void loadQuestions(QVBoxLayout *contentLayout);
    void addQuestionToPanel(QVBoxLayout *contentLayout, const QString &questionText, const QString &questionType, int questionId);

    void onDatabasePathChanged();
};

#endif // MAINWINDOW_H
