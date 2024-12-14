#include "sidebarjoblistwidget.h"
#include "jobpreviewwidget.h"
#include "qboxlayout.h"
#include <QListWidgetItem>
#include <QDebug>

SidebarJobListWidget::SidebarJobListWidget(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), dbManager(dbManager)
{
    qDebug() << "SidebarJobListWidget parent during creation:" << parent;
    // Initialize List Widget
    jobListWidget = new QListWidget(this);
    jobListWidget->setSpacing(5); // Add spacing between items
    jobListWidget->setFrameStyle(QFrame::NoFrame); // Remove border
    jobListWidget->setResizeMode(QListWidget::Adjust); // Adjust size to content
    jobListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Connect Item Click Signal
    connect(jobListWidget, &QListWidget::itemClicked, this, &SidebarJobListWidget::handleItemClick);

    // Load Jobs from Database
    loadJobs();

    // Set Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(jobListWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void SidebarJobListWidget::loadJobs()
{
    // Fetch jobs from DatabaseManager
    jobList = dbManager->getJobs();

    for (int i = 0; i < jobList.size(); ++i) {
        const Job &job = jobList[i];

        // Create a custom widget for each job preview
        JobPreviewWidget *previewWidget = new JobPreviewWidget(job.jobTitle, job.companyName, job.appSubmitted, job.preferenceScore, this);

        // Create a QListWidgetItem and set its size
        QListWidgetItem *item = new QListWidgetItem(jobListWidget);
        item->setSizeHint(previewWidget->sizeHint());

        // Add the custom widget to the list
        jobListWidget->addItem(item);
        jobListWidget->setItemWidget(item, previewWidget);

        // Store job index in the item for selection handling
        item->setData(Qt::UserRole, i);
    }
}

QList<Job>& SidebarJobListWidget::getJobs()
{
    return jobList;
}

void SidebarJobListWidget::selectJobByIndex(int index)
{
    if (index >= 0 && index < jobListWidget->count()) {
        jobListWidget->setCurrentRow(index); // Update the selected row
        emit jobSelected(index);            // Emit the selection signal
    }
}

void SidebarJobListWidget::handleItemClick(QListWidgetItem *item)
{
    // Safely cast the parent to MainWindow
    emit jobListingRequested(); // Emit the signal

    // Emit jobSelected signal with the index of the clicked item
    int index = item->data(Qt::UserRole).toInt();
    emit jobSelected(index);
}

