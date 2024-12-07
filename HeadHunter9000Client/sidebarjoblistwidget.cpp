#include "sidebarjoblistwidget.h"
#include "jobpreviewwidget.h"
#include <QListWidgetItem>
#include <QDebug>

SidebarJobListWidget::SidebarJobListWidget(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), dbManager(dbManager)
{
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
    QList<Job> jobs = dbManager->getJobs();

    for (const Job &job : jobs) {
        // Create a custom widget for each job preview
        JobPreviewWidget *previewWidget = new JobPreviewWidget(job.jobTitle, job.companyName, this);

        // Create a QListWidgetItem and set its size
        QListWidgetItem *item = new QListWidgetItem(jobListWidget);
        item->setSizeHint(previewWidget->sizeHint());

        // Add the custom widget to the list
        jobListWidget->addItem(item);
        jobListWidget->setItemWidget(item, previewWidget);

        // Store job ID in the item for selection handling
        item->setData(Qt::UserRole, job.id);
    }
}

void SidebarJobListWidget::handleItemClick(QListWidgetItem *item)
{
    // Emit signal with the job ID of the clicked item
    int jobId = item->data(Qt::UserRole).toInt();
    emit jobSelected(jobId);

    qDebug() << "Job ID" << jobId << "selected.";
}
