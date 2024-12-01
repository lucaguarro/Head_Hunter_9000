#include "starratingwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>

StarRatingWidget::StarRatingWidget(QWidget *parent)
    : QWidget(parent)
    , m_rating(0)
{
    setMinimumSize(100, 20);
}

void StarRatingWidget::setRating(int rating)
{
    if (rating != m_rating && rating >= 0 && rating <= 5) {
        m_rating = rating;
        update();
        emit ratingChanged(m_rating);
    }
}

int StarRatingWidget::rating() const
{
    return m_rating;
}

void StarRatingWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    int starWidth = width() / 5;
    int starHeight = height();

    QPixmap filledStar(":/images/filled_star.png"); // Ensure this path is correct
    QPixmap emptyStar(":/images/empty_star.png");   // Ensure this path is correct

    for (int i = 0; i < 5; ++i) {
        QRect starRect(i * starWidth, 0, starWidth, starHeight);
        if (i < m_rating) {
            painter.drawPixmap(starRect, filledStar);
        } else {
            painter.drawPixmap(starRect, emptyStar);
        }
    }
}

void StarRatingWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int starWidth = width() / 5;
    int clickedStar = event->x() / starWidth + 1;
    if (clickedStar >= 1 && clickedStar <= 5) {
        setRating(clickedStar);
    }
}
