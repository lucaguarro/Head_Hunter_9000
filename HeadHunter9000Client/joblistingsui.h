#ifndef JOBLISTINGSUI_H
#define JOBLISTINGSUI_H

#include <QWidget>
#include "sidebarjoblistwidget.h"

class QLabel;
class QPushButton;
class StarRatingWidget;

class JobListingsUI : public QWidget
{
    Q_OBJECT

public:
    explicit JobListingsUI(QWidget *parent = nullptr, DatabaseManager *dbManager = nullptr, SidebarJobListWidget *sidebar = nullptr);

private slots:
    void showPreviousJob();
    void showNextJob();
    void displayCurrentJob();
    void updatePreferenceScore(int score);
    void handleSidebarSelection(int index); // Handle job selection from sidebar

private:
    DatabaseManager *dbManager;
    SidebarJobListWidget *sidebar;
    int currentIndex;

    QLabel *companyNameLabel;
    QLabel *locationLabel;
    QLabel *jobTitleLabel;
    QLabel *descriptionLabel;
    QLabel *createdAtLabel;
    QPushButton *prevButton;
    QPushButton *nextButton;
    StarRatingWidget *starRatingWidget;

    void setupUI();

};

#endif // JOBLISTINGSUI_H
