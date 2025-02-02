#ifndef BUTTONGROUPBOX_H
#define BUTTONGROUPBOX_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ButtonGroupBox : public QWidget
{
    Q_OBJECT
public:
    explicit ButtonGroupBox(const QString &title = QString(),
                            QWidget *parent = nullptr)
        : QWidget(parent)
        , m_titleLabel(new QLabel(title, this))
        , m_button(new QPushButton("...", this))
        , m_contentFrame(new QFrame(this))
        , m_contentLayout(new QVBoxLayout())
    {
        //
        // 1) Overall vertical layout
        //
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        //
        // 2) Top row: label + stretch + button
        //
        QHBoxLayout *titleLayout = new QHBoxLayout();
        // Make label bold to mimic a typical group-box label
        QFont f = m_titleLabel->font();
        m_titleLabel->setFont(f);

        titleLayout->addWidget(m_titleLabel);
        titleLayout->addStretch();
        titleLayout->addWidget(m_button);

        mainLayout->addLayout(titleLayout);

        //
        // 3) Framed content area
        //
        // Style the frame like a simple box/panel so it looks like a group box
        m_contentFrame->setFrameShape(QFrame::StyledPanel);
        m_contentFrame->setFrameShadow(QFrame::Plain);

        // Put m_contentLayout inside the frame
        m_contentFrame->setLayout(m_contentLayout);
        mainLayout->addWidget(m_contentFrame);
    }

    void setTitle(const QString &title)
    {
        m_titleLabel->setText(title);
    }

    QString title() const
    {
        return m_titleLabel->text();
    }

    void setButtonText(const QString &text)
    {
        m_button->setText(text);
    }

    QString buttonText() const
    {
        return m_button->text();
    }

    // Expose the button if needed (for signals/slots)
    QPushButton *button() const
    {
        return m_button;
    }

    // Layout for content that goes inside the "grey box"
    QVBoxLayout *contentLayout() const
    {
        return m_contentLayout;
    }

private:
    QLabel      *m_titleLabel;
    QPushButton *m_button;
    QFrame      *m_contentFrame;
    QVBoxLayout *m_contentLayout;
};

#endif // BUTTONGROUPBOX_H
