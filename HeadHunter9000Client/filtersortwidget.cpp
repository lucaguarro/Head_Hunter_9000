#include "filtersortwidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QDebug>

FilterSortWidget::FilterSortWidget(const QDateTime &minDate,
                                   const QDateTime &maxDate,
                                   QWidget *parent)
    : QWidget(parent),
    minDate(minDate),
    maxDate(maxDate)
{
    setWindowFlags(Qt::Popup);
    setObjectName("FilterSortContainer");

    // Slight background & border on the container, minimal padding for child widgets
    setStyleSheet(R"(
        #FilterSortContainer {
            background-color: #ffffff;
            border: 1px solid #cccccc;
            border-radius: 4px;
        }
        #FilterSortContainer QComboBox,
        #FilterSortContainer QCheckBox,
        #FilterSortContainer QPushButton,
        #FilterSortContainer QLabel {
            padding: 2px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // Title: Filter by Applied Status
    QLabel *appliedTitle = new QLabel("<b>Filter by Applied Status</b>", this);
    appliedCombo = new QComboBox(this);
    appliedCombo->addItem("All", 0);
    appliedCombo->addItem("Applied Only", 1);
    appliedCombo->addItem("Not Applied", 2);
    mainLayout->addWidget(appliedTitle);
    mainLayout->addWidget(appliedCombo);

    // Rating
    hasRatingCheck = new QCheckBox("Has rating?", this);
    mainLayout->addWidget(hasRatingCheck);

    // Scraped Date row
    QHBoxLayout *scrapedLayout = new QHBoxLayout();
    QLabel *scrapedTitle = new QLabel("Scraped Date:", this);
    scrapedDateSlider = new QSlider(Qt::Horizontal, this);
    scrapedDateLabel = new QLabel(maxDate.toString("yyyy-MM-dd"), this);

    scrapedLayout->addWidget(scrapedTitle);
    scrapedLayout->addWidget(scrapedDateSlider);
    scrapedLayout->addWidget(scrapedDateLabel);
    mainLayout->addLayout(scrapedLayout);

    // Sort By
    QLabel *sortTitle = new QLabel("<b>Sort By</b>", this);
    sortCombo = new QComboBox(this);
    sortCombo->addItem("Sort by Rating", "preferenceScore");
    sortCombo->addItem("Sort by Applied", "appSubmitted");
    sortCombo->addItem("Sort by Created At", "createdAt");
    sortCombo->addItem("Sort by Job Title", "jobTitle");
    sortCombo->addItem("Sort by Company", "companyName");

    mainLayout->addWidget(sortTitle);
    mainLayout->addWidget(sortCombo);

    // Ascending checkbox
    ascendingCheck = new QCheckBox("Ascending", this);
    ascendingCheck->setChecked(true); // default to ascending
    mainLayout->addWidget(ascendingCheck);

    // Apply button
    applyButton = new QPushButton("Apply", this);
    connect(applyButton, &QPushButton::clicked, this, &FilterSortWidget::applyClicked);
    mainLayout->addWidget(applyButton);

    setLayout(mainLayout);

    // Initialize the slider range
    refreshDateSlider();
    connect(scrapedDateSlider, &QSlider::valueChanged, this, &FilterSortWidget::updateSliderLabel);
}

void FilterSortWidget::updateDateRange(const QDateTime &newMinDate, const QDateTime &newMaxDate)
{
    minDate = newMinDate;
    maxDate = newMaxDate;

    refreshDateSlider();
}

void FilterSortWidget::refreshDateSlider()
{
    qint64 dayDiff = minDate.daysTo(maxDate);
    dayDiff = qMax<qint64>(dayDiff, 0);

    scrapedDateSlider->blockSignals(true); // prevent emitting signals temporarily
    scrapedDateSlider->setRange(0, dayDiff);
    scrapedDateSlider->setValue(dayDiff); // default to max (latest date)
    scrapedDateSlider->blockSignals(false);

    scrapedDateLabel->setText(maxDate.toString("yyyy-MM-dd"));
}

void FilterSortWidget::updateSliderLabel(int sliderValue)
{
    QDateTime selectedDate = minDate.addDays(sliderValue);
    scrapedDateLabel->setText(selectedDate.toString("yyyy-MM-dd"));
}

void FilterSortWidget::applyClicked()
{
    // 1) Applied status
    int appliedMode = appliedCombo->currentData().toInt();
    emit appliedFilterChanged(appliedMode);

    // 2) Rating filter
    bool hasRating = hasRatingCheck->isChecked();
    emit ratingFilterChanged(hasRating);

    // 3) Date slider => compute actual QDateTime
    int sliderValue = scrapedDateSlider->value();
    QDateTime cutoffDate = minDate.addDays(sliderValue);
    emit dateCutoffChanged(cutoffDate);

    // 4) Sorting
    QString sortField = sortCombo->currentData().toString();
    bool ascending = ascendingCheck->isChecked();  // read from the checkbox
    emit sortChanged(sortField, ascending);

    hide();
}

void FilterSortWidget::reset(const QDateTime &newMinDate, const QDateTime &newMaxDate)
{
    minDate = newMinDate;
    maxDate = newMaxDate;

    // Update the slider to the new range
    refreshDateSlider();

    // Reset UI controls to defaults
    appliedCombo->setCurrentIndex(0);         // "All"
    hasRatingCheck->setChecked(false);        // Not filtered by rating
    scrapedDateSlider->setValue(scrapedDateSlider->maximum()); // Latest date
    sortCombo->setCurrentIndex(0);            // Sort by Rating
    ascendingCheck->setChecked(true);         // Ascending

    // Update label manually
    updateSliderLabel(scrapedDateSlider->value());

    // Optionally emit signals immediately if you want to auto-refresh the filtering
    emit appliedFilterChanged(appliedCombo->currentData().toInt());
    emit ratingFilterChanged(hasRatingCheck->isChecked());
    emit dateCutoffChanged(minDate.addDays(scrapedDateSlider->value()));
    emit sortChanged(sortCombo->currentData().toString(), ascendingCheck->isChecked());
}
