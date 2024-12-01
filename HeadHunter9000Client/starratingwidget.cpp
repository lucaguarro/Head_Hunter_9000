#include "starratingwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <QDebug>

StarRatingWidget::StarRatingWidget(QWidget *parent)
    : QWidget(parent)
    , m_rating(0)
{
    setMinimumSize(100, 30); // Adjusted for better visibility
    // setMaximumHeight(30);
    setStyleSheet("background-color: lightgray;"); // Temporary background color for debugging
}

void StarRatingWidget::setRating(int rating)
{
    if (rating != m_rating && rating >= 0 && rating <= 5) {
        m_rating = rating;
        update(); // Trigger repaint
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

    // Initialize SVG Renderers with explicit parent
    QSvgRenderer rendererFilled(QStringLiteral(":/icons/assets/icons/star-solid.svg"), this);
    QSvgRenderer rendererEmpty(QStringLiteral(":/icons/assets/icons/star-regular.svg"), this);

    // Check if SVGs are loaded correctly
    if (!rendererFilled.isValid()) {
        qWarning() << "Failed to load filled star SVG.";
    }
    if (!rendererEmpty.isValid()) {
        qWarning() << "Failed to load empty star SVG.";
    }

    for (int i = 0; i < 5; ++i) {
        QRect starRect(i * starWidth, 0, starWidth, starHeight);
        if (i < m_rating) {
            rendererFilled.render(&painter, starRect);
        } else {
            rendererEmpty.render(&painter, starRect);
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
