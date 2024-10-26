#include "checkablecombobox.h"
#include <QLineEdit>

CheckableComboBox::CheckableComboBox(QWidget* parent)
    : QComboBox(parent)
{
    setEditable(true);               // Enable editing to display custom text
    lineEdit()->setReadOnly(true);   // Prevent user from editing the text
}

void CheckableComboBox::setMenu(QMenu* menu)
{
    m_menu = menu;
}

void CheckableComboBox::showPopup()
{
    if (m_menu) {
        // Set the menu width to match the combo box width
        m_menu->setFixedWidth(width());

        // Optional: Adjust the position if needed (e.g., for margins)
        QPoint pos = mapToGlobal(QPoint(0, height()));
        m_menu->exec(pos);
    } else {
        QComboBox::showPopup();
    }
}
