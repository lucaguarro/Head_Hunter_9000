#include "jobpreviewwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QSizePolicy>
#include <QVBoxLayout>

JobPreviewWidget::JobPreviewWidget(const QString &jobTitle,
                                   const QString &companyName,
                                   bool isApplied,
                                   bool isRated,
                                   QWidget *parent)
    : QWidget(parent)
{
    // Create labels
    jobTitleLabel = new QLabel(jobTitle, this);
    jobTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: black;");
    // Ensure the label can elide (truncate) text if not enough space
    jobTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    jobTitleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    jobTitleLabel->setWordWrap(false);
    jobTitleLabel->setTextElideMode(Qt::ElideRight);

    companyNameLabel = new QLabel(companyName, this);
    companyNameLabel->setStyleSheet("font-size: 12px; color: gray;");
    companyNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    companyNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    companyNameLabel->setWordWrap(false);

    // Create icon labels for applied (check) and rating (star)
    appliedIconLabel = new QLabel(this);
    ratingIconLabel = new QLabel(this);

    // If the job is applied, show the green check icon; otherwise hide
    if (isApplied) {
        appliedIconLabel->setPixmap(QPixmap(":/icons/assets/icons/check-solid.svg")
                                        .scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        appliedIconLabel->hide();  // No icon if not applied
    }

    // If the job is rated, show the filled star; otherwise show the empty star
    if (isRated) {
        ratingIconLabel->setPixmap(QPixmap(":/icons/assets/icons/star-solid.svg")
                                       .scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ratingIconLabel->setPixmap(QPixmap(":/icons/assets/icons/star-regular.svg")
                                       .scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // Top row layout (job title + applied check)
    QHBoxLayout *topRowLayout = new QHBoxLayout;
    // Stretch factor: text label gets “1” so it expands first; icon gets “0” so it remains fixed.
    topRowLayout->addWidget(jobTitleLabel, /*stretch=*/1);
    topRowLayout->addWidget(appliedIconLabel, /*stretch=*/0);

    // Bottom row layout (company name + rating star)
    QHBoxLayout *bottomRowLayout = new QHBoxLayout;
    bottomRowLayout->addWidget(companyNameLabel, /*stretch=*/1);
    bottomRowLayout->addWidget(ratingIconLabel, /*stretch=*/0);

    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topRowLayout);
    mainLayout->addLayout(bottomRowLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(2);

    setLayout(mainLayout);
}
