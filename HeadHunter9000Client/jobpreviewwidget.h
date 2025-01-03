#ifndef JOBPREVIEWWIDGET_H
#define JOBPREVIEWWIDGET_H

#include "elidedlabel.h"
#include <QWidget>

class QLabel;

class JobPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JobPreviewWidget(const QString &jobTitle,
                              const QString &companyName,
                              bool isApplied = false,
                              bool isRated = false,
                              QWidget *parent = nullptr);

private:
    ElidedLabel *jobTitleLabel;
    ElidedLabel *companyNameLabel;
    QLabel *appliedIconLabel;  // icon in top-right
    QLabel *ratingIconLabel;   // icon in bottom-right
};

#endif // JOBPREVIEWWIDGET_H
