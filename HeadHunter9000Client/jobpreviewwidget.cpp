#include "jobpreviewwidget.h"

JobPreviewWidget::JobPreviewWidget(const QString &jobTitle, const QString &companyName, QWidget *parent)
    : QWidget(parent)
{
    // Initialize Labels
    jobTitleLabel = new QLabel(jobTitle, this);
    jobTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: black;");

    companyNameLabel = new QLabel(companyName, this);
    companyNameLabel->setStyleSheet("font-size: 12px; color: gray;");

    // Create Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(jobTitleLabel);
    layout->addWidget(companyNameLabel);
    layout->setContentsMargins(5, 5, 5, 5); // Add padding around items
    layout->setSpacing(2);

    setLayout(layout);
}
