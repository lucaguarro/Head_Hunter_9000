#ifndef JOBLISTINGSUI_H
#define JOBLISTINGSUI_H

#include <QWidget>
#include <QList>
#include "job.h"
#include "starratingwidget.h" // Include the StarRatingWidget header

class DatabaseManager;
class QLabel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QScrollArea; // Include QScrollArea

class JobListingsUI : public QWidget
{
    Q_OBJECT
public:
    explicit JobListingsUI(QWidget *parent = nullptr, DatabaseManager *dbManager = nullptr);

private slots:
    void showPreviousJob();
    void showNextJob();
    void updatePreferenceScore(int score);

private:
    void setupUI();
    void loadJobs();
    void displayCurrentJob();

    DatabaseManager *dbManager;
    QList<Job> jobList;
    int currentIndex;

    QLabel *companyNameLabel;
    QLabel *locationLabel;
    QLabel *jobTitleLabel;
    QLabel *descriptionLabel;
    QLabel *createdAtLabel;
    StarRatingWidget *starRatingWidget; // Use your custom StarRatingWidget
    QPushButton *prevButton;
    QPushButton *nextButton;

    QScrollArea *descriptionScrollArea; // Scroll area for description
};

#endif // JOBLISTINGSUI_H
