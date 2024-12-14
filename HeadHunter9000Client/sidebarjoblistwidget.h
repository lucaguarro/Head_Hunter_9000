#ifndef SIDEBARJOBLISTWIDGET_H
#define SIDEBARJOBLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "databasemanager.h"

class SidebarJobListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarJobListWidget(DatabaseManager *dbManager, QWidget *parent = nullptr);
    QList<Job>& getJobs(); // Expose the job list
    void selectJobByIndex(int index); // Select a job by index programmatically

signals:
    void jobSelected(int index); // Signal emitted when a job is selected
    void jobListingRequested();

private slots:
    void handleItemClick(QListWidgetItem *item);

private:
    QListWidget *jobListWidget;
    DatabaseManager *dbManager;
    QList<Job> jobList; // Store the job list

    void loadJobs();
};

#endif // SIDEBARJOBLISTWIDGET_H
