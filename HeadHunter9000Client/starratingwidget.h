// starratingwidget.h

#ifndef STARRATINGWIDGET_H
#define STARRATINGWIDGET_H

#include <QWidget>

class StarRatingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StarRatingWidget(QWidget *parent = nullptr);

    void setRating(int rating, bool updateDB);
    int rating() const;

signals:
    void ratingChanged(int newRating);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int m_rating;
};

#endif // STARRATINGWIDGET_H
