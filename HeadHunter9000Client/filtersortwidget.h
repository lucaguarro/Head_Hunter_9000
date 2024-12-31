#pragma once

#include <QWidget>
#include <QDateTime>

class QCheckBox;
class QComboBox;
class QSlider;
class QLabel;
class QPushButton;

class FilterSortWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterSortWidget(const QDateTime &minDate,
                              const QDateTime &maxDate,
                              QWidget *parent = nullptr);

signals:
    void appliedFilterChanged(int appliedMode);
    void ratingFilterChanged(bool hasRating);
    void dateCutoffChanged(const QDateTime &cutoffDate);
    void sortChanged(const QString &sortField, bool ascending);

private slots:
    void updateSliderLabel(int sliderValue);
    void applyClicked();

private:
    QComboBox *appliedCombo;
    QCheckBox *hasRatingCheck;
    QSlider   *scrapedDateSlider;
    QLabel    *scrapedDateLabel;
    QPushButton *applyButton;
    QComboBox *sortCombo;

    // Instead of a second combo, we use a single checkbox for ascending
    QCheckBox *ascendingCheck;

    QDateTime minDate, maxDate;
};
