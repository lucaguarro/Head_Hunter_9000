#pragma once

#include <QWidget>
#include <QListWidget>
#include <QDateTime>
#include "databasemanager.h"
#include "filtersortwidget.h"

class SidebarJobListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarJobListWidget(DatabaseManager *dbManager, QWidget *parent = nullptr);

    QList<Job>& getJobs();
    void selectJobByIndex(int index);

signals:
    void jobSelected(int index);
    void jobListingRequested();

private slots:
    void handleItemClick(QListWidgetItem *item);
    void handleCurrentRowChanged(int currentRow);

    // Called by FilterSortWidget signals
    void applyAppliedFilter(int appliedMode);
    void applyRatingFilter(bool hasRating);
    void applyDateCutoff(const QDateTime &cutoffDate);
    void applySort(const QString &sortField, bool ascending);

private:
    void loadJobs();
    void populateList(const QList<Job> &jobs);
    void showFilterSortPopup();

    DatabaseManager *dbManager;
    QList<Job> jobList;      // Original list from DB
    QList<Job> filteredJobs; // After filtering and sorting

    QListWidget *jobListWidget;
    QPushButton *filterSortButton;
    FilterSortWidget *filterSortWidget;

    // We store the currently chosen filter states:
    int appliedMode;        // 0=All,1=AppliedOnly,2=NotApplied
    bool ratingFilterOn;    // Show only jobs with rating?
    QDateTime dateCutoff;   // Only show jobs >= cutoff
};
