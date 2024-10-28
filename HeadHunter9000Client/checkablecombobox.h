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
    QMenu* m_menu = nullptr;

protected:
    void showPopup() override;

private:

};

#endif // CHECKABLECOMBOBOX_H
