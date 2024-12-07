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

signals:
    void jobSelected(int jobId); // Signal emitted when a job is clicked

private slots:
    void handleItemClick(QListWidgetItem *item);

private:
    QListWidget *jobListWidget;
    DatabaseManager *dbManager;

    void loadJobs();
};

#endif // SIDEBARJOBLISTWIDGET_H
