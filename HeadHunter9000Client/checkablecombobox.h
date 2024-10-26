#ifndef CHECKABLECOMBOBOX_H
#define CHECKABLECOMBOBOX_H

#include <QComboBox>
#include <QMenu>

class CheckableComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CheckableComboBox(QWidget* parent = nullptr);

    void setMenu(QMenu* menu);

protected:
    void showPopup() override;

private:
    QMenu* m_menu = nullptr;
};

#endif // CHECKABLECOMBOBOX_H
