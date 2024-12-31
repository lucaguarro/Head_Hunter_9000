#include "sidebarjoblistwidget.h"
#include "jobpreviewwidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidgetItem>
#include <QDebug>
#include <algorithm>

SidebarJobListWidget::SidebarJobListWidget(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent),
    dbManager(dbManager),
    appliedMode(0),
    ratingFilterOn(false)
{
    jobListWidget = new QListWidget(this);
    jobListWidget->setSpacing(5);
    jobListWidget->setFrameStyle(QFrame::NoFrame);
    jobListWidget->setResizeMode(QListWidget::Adjust);
    jobListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(jobListWidget, &QListWidget::itemClicked, this, &SidebarJobListWidget::handleItemClick);
    connect(jobListWidget, &QListWidget::currentRowChanged, this, &SidebarJobListWidget::handleCurrentRowChanged);

    // Load all jobs
    loadJobs();

    // Determine the oldest & newest date for the slider
    QDateTime minDate = QDateTime::currentDateTime();
    QDateTime maxDate = QDateTime::fromString("1970-01-01", Qt::ISODate);
    for (const Job &job : jobList) {
        QDateTime dt = QDateTime::fromString(job.createdAt, Qt::ISODate);
        if (dt.isValid()) {
            if (dt < minDate) minDate = dt;
            if (dt > maxDate) maxDate = dt;
        }
    }
    // Fallback if no valid dates
    if (!minDate.isValid())  minDate = QDateTime::currentDateTime().addYears(-1);
    if (!maxDate.isValid())  maxDate = QDateTime::currentDateTime();

    // Create the Filter/Sort pop-up
    filterSortWidget = new FilterSortWidget(minDate, maxDate, this);
    filterSortWidget->hide();
    connect(filterSortWidget, &FilterSortWidget::appliedFilterChanged,
            this, &SidebarJobListWidget::applyAppliedFilter);
    connect(filterSortWidget, &FilterSortWidget::ratingFilterChanged,
            this, &SidebarJobListWidget::applyRatingFilter);
    connect(filterSortWidget, &FilterSortWidget::dateCutoffChanged,
            this, &SidebarJobListWidget::applyDateCutoff);
    connect(filterSortWidget, &FilterSortWidget::sortChanged,
            this, &SidebarJobListWidget::applySort);


    // Default dateCutoff is the max date => show everything
    dateCutoff = maxDate;

    // Populate initially (no filters, no sort)
    filteredJobs = jobList;
    populateList(filteredJobs);

    // Filter/Sort button
    filterSortButton = new QPushButton("Filter/Sort", this);
    connect(filterSortButton, &QPushButton::clicked, this, &SidebarJobListWidget::showFilterSortPopup);

    // Layout
    // ---- Create a header bar ----
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *sidebarTitle = new QLabel("Your Jobs", this);
    sidebarTitle->setStyleSheet("font-weight: bold; font-size: 16px;");


    headerLayout->addWidget(sidebarTitle);
    headerLayout->addStretch(); // Push the button to the right
    headerLayout->addWidget(filterSortButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(headerLayout); // The new header bar
    mainLayout->addWidget(jobListWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);
}

void SidebarJobListWidget::loadJobs()
{
    // Original unfiltered data
    jobList = dbManager->getJobs();
}

void SidebarJobListWidget::populateList(const QList<Job> &jobs)
{
    jobListWidget->clear();

    for (int i = 0; i < jobs.size(); ++i) {
        const Job &job = jobs[i];
        JobPreviewWidget *previewWidget = new JobPreviewWidget(job.jobTitle, job.companyName,
                                                               job.appSubmitted, job.preferenceScore,
                                                               this);

        QListWidgetItem *item = new QListWidgetItem(jobListWidget);
        item->setSizeHint(previewWidget->sizeHint());
        jobListWidget->addItem(item);
        jobListWidget->setItemWidget(item, previewWidget);

        // Store the index within filteredJobs
        item->setData(Qt::UserRole, i);
    }
}

QList<Job>& SidebarJobListWidget::getJobs()
{
    // Return filtered list to the rest of the UI
    return filteredJobs;
}

void SidebarJobListWidget::selectJobByIndex(int index)
{
    if (index >= 0 && index < jobListWidget->count()) {
        jobListWidget->setCurrentRow(index);
        emit jobSelected(index);
    }
}

void SidebarJobListWidget::handleItemClick(QListWidgetItem *item)
{
    emit jobListingRequested();

    int index = item->data(Qt::UserRole).toInt();
    emit jobSelected(index);
}

void SidebarJobListWidget::handleCurrentRowChanged(int currentRow)
{
    if (currentRow >= 0 && currentRow < filteredJobs.size()) {
        QListWidgetItem *item = jobListWidget->item(currentRow);
        if (!item) return;
        int index = item->data(Qt::UserRole).toInt();
        emit jobSelected(index);
    }
}

void SidebarJobListWidget::showFilterSortPopup()
{
    // Position the pop-up at the top-left corner of the sidebar, below the header
    QPoint globalPos = mapToGlobal(QPoint(0, filterSortButton->geometry().bottom()));

    // The current width of the SidebarJobListWidget
    int sidebarWidth = this->width();
    // We impose a minimum of 300 px
    int minPopupWidth = 300;

    // The actual width for the popup is the max of sidebarWidth or minPopupWidth
    int popupWidth = qMax(sidebarWidth, minPopupWidth);

    // Keep the existing popup height or a default if needed
    int popupHeight = filterSortWidget->height();
    // Or set a minimum height as well if you have a lot of filter controls:
    // popupHeight = qMax(popupHeight, 200);

    filterSortWidget->setMinimumSize(minPopupWidth, 250);
    // This ensures if the user tries to resize the popup, it won't go below 300x200

    filterSortWidget->resize(popupWidth, popupHeight);

    // Move it into position
    filterSortWidget->move(globalPos.x(), globalPos.y());

    filterSortWidget->show();
    filterSortWidget->raise();
    filterSortWidget->activateWindow();
}



// ------------------ FILTER SLOTS ------------------

void SidebarJobListWidget::applyAppliedFilter(int mode)
{
    // 0 = All, 1 = Applied Only, 2 = Not Applied
    appliedMode = mode;
}

void SidebarJobListWidget::applyRatingFilter(bool hasRating)
{
    ratingFilterOn = hasRating;
}

void SidebarJobListWidget::applyDateCutoff(const QDateTime &cutoffDate)
{
    dateCutoff = cutoffDate;
}

// When any of these change, we want to re-apply the filters & re-populate
void SidebarJobListWidget::applySort(const QString &sortField, bool ascending)
{
    // 1) Start from the original jobList
    filteredJobs = jobList;

    // 2) Apply filters

    // a) Applied filter
    if (appliedMode == 1) {
        // Show only applied
        auto it = std::remove_if(filteredJobs.begin(), filteredJobs.end(),
                                 [](const Job &j){ return j.appSubmitted == false; });
        filteredJobs.erase(it, filteredJobs.end());
    } else if (appliedMode == 2) {
        // Show only NOT applied
        auto it = std::remove_if(filteredJobs.begin(), filteredJobs.end(),
                                 [](const Job &j){ return j.appSubmitted == true; });
        filteredJobs.erase(it, filteredJobs.end());
    }
    // 0 => All, do nothing

    // b) Rating filter (hasRating==true means preferenceScore != 0)
    if (ratingFilterOn) {
        auto it = std::remove_if(filteredJobs.begin(), filteredJobs.end(),
                                 [](const Job &j){ return j.preferenceScore == 0; });
        filteredJobs.erase(it, filteredJobs.end());
    }

    // c) Date cutoff filter: keep only jobs whose createdAt >= dateCutoff
    auto it = std::remove_if(filteredJobs.begin(), filteredJobs.end(),
                             [this](const Job &j){
                                 QDateTime jobDate = QDateTime::fromString(j.createdAt, Qt::ISODate);
                                 if (!jobDate.isValid()) return true; // filter out invalid
                                 return jobDate < dateCutoff;
                             });
    filteredJobs.erase(it, filteredJobs.end());

    // 3) Sorting (now using ascending/descending logic)
    if (sortField == "preferenceScore") {
        std::sort(filteredJobs.begin(), filteredJobs.end(),
                  [ascending](const Job &a, const Job &b){
                      // If ascending, a < b. If descending, a > b.
                      return ascending ? (a.preferenceScore < b.preferenceScore)
                                       : (a.preferenceScore > b.preferenceScore);
                  });
    } else if (sortField == "appSubmitted") {
        std::sort(filteredJobs.begin(), filteredJobs.end(),
                  [ascending](const Job &a, const Job &b){
                      // For a bool, ascending means false -> true
                      // So ascending => (a < b). Desc => (a > b).
                      return ascending ? (a.appSubmitted < b.appSubmitted)
                                       : (a.appSubmitted > b.appSubmitted);
                  });
    } else if (sortField == "createdAt") {
        std::sort(filteredJobs.begin(), filteredJobs.end(),
                  [ascending](const Job &a, const Job &b){
                      return ascending ? (a.createdAt < b.createdAt)
                                       : (a.createdAt > b.createdAt);
                  });
    } else if (sortField == "jobTitle") {
        std::sort(filteredJobs.begin(), filteredJobs.end(),
                  [ascending](const Job &a, const Job &b){
                      int cmp = a.jobTitle.compare(b.jobTitle, Qt::CaseInsensitive);
                      return ascending ? (cmp < 0) : (cmp > 0);
                  });
    } else if (sortField == "companyName") {
        std::sort(filteredJobs.begin(), filteredJobs.end(),
                  [ascending](const Job &a, const Job &b){
                      int cmp = a.companyName.compare(b.companyName, Qt::CaseInsensitive);
                      return ascending ? (cmp < 0) : (cmp > 0);
                  });
    }

    // 4) Repopulate the list
    populateList(filteredJobs);
}

