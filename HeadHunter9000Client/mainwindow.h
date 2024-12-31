#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QPointer>

class DatabaseManager;
class SidebarJobListWidget;
class QPushButton;
class ProcessWorker;
class QThread;
// Forward declarations of your UI widgets
class JobListingsUI;
class AskQuestionsUI;
class SeeAllQuestionsUI;
class ScraperConfigurationUI;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSidebarButtonClicked(QPushButton* clickedButton, const QList<QPushButton*>& buttons);
    void on_ScraperConfigBtn_clicked();
    void on_SeeAllQuestionsBtn_clicked();
    void on_AnswerQuestionsBtn_clicked();
    void on_ExecuteBtn_clicked();
    void onDatabasePathChanged();
    void createJobListingsUI();

private:
    Ui::MainWindow *ui;
    DatabaseManager *dbManager;
    SidebarJobListWidget *sidebarjoblistwidget;
    QSettings *settings;

    // Threading bits
    QThread *thread = nullptr;
    ProcessWorker *worker = nullptr;
    bool isProcessRunning = false;

    // Use ONE pointer to track the current widget in mainAreaContainer
    QWidget *currentWidget = nullptr;

    // We still keep track of which sidebar button was previously clicked
    QPushButton *previousButton;

    // Utility function to set the main widget properly
    void setMainWidget(QWidget *newWidget);

    void setExecutionStateUI();
};

#endif // MAINWINDOW_H
