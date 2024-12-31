#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>
#include <QResizeEvent>

class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent), m_fullText(text)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setWordWrap(false);
        // Optionally:
        // setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    }

    void setFullText(const QString &text) {
        m_fullText = text;
        updateElidedText();
    }

    QString fullText() const { return m_fullText; }

    void setElideMode(Qt::TextElideMode mode) {
        m_elideMode = mode;
        updateElidedText();
    }

    Qt::TextElideMode elideMode() const { return m_elideMode; }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QLabel::resizeEvent(event);
        updateElidedText();
    }

private:
    void updateElidedText() {
        if (width() <= 0) {
            return;
        }
        QFontMetrics fm(font());
        QString elided = fm.elidedText(m_fullText, m_elideMode, width());
        QLabel::setText(elided);
    }

    QString m_fullText;
    Qt::TextElideMode m_elideMode { Qt::ElideRight };
};

#endif // ELIDEDLABEL_H
