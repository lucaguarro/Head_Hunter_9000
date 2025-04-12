#ifndef PREFERENCESDIALOGUI_H
#define PREFERENCESDIALOGUI_H

#include <QDialog>

class QTreeWidget;
class QStackedWidget;

class PreferencesDialogUI : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialogUI(QWidget *parent = nullptr);

private:
    QTreeWidget *navigationTree;
    QStackedWidget *stackedWidget;

    void setupUI();
    void setupNavigation();
};

#endif // PREFERENCESDIALOGUI_H
