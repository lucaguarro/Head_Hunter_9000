#ifndef JOBPREVIEWWIDGET_H
#define JOBPREVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class JobPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JobPreviewWidget(const QString &jobTitle, const QString &companyName, QWidget *parent = nullptr);

private:
    QLabel *jobTitleLabel;
    QLabel *companyNameLabel;
};

#endif // JOBPREVIEWWIDGET_H
