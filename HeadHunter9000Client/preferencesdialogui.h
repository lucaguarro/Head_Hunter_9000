#ifndef PREFERENCESDIALOGUI_H
#define PREFERENCESDIALOGUI_H

#include <QDialog>

class QListWidget;
class QStackedWidget;

class PreferencesDialogUI : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialogUI(QWidget *parent = nullptr);

private:
    QListWidget *sidePanel;
    QStackedWidget *stackedWidget;

    void setupUI();
    void createPages();
};

#endif // PREFERENCESDIALOGUI_H
