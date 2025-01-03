#include "starratingwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <QDebug>

StarRatingWidget::StarRatingWidget(QWidget *parent)
    : QWidget(parent)
    , m_rating(0)
{
    setMinimumSize(100, 20); // Width: 100px, Height: 20px
    setMaximumSize(150, 20); // Allow minor expansion in width
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Fixed height
}

void StarRatingWidget::setRating(int rating, bool updateDB=true)
{
    if (rating != m_rating && rating >= 0 && rating <= 5) {
        m_rating = rating;
        update(); // Trigger repaint
        if (updateDB){
            emit ratingChanged(m_rating);
        }
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
    int clickedStar = static_cast<int>(event->position().x()) / starWidth + 1;
    if (clickedStar >= 1 && clickedStar <= 5) {
        setRating(clickedStar);
    }
}
