#ifndef PREFERENCESDIALOGUI_H
#define PREFERENCESDIALOGUI_H

#include <QSettings>
#include <QDialog>

class QTreeWidget;
class QStackedWidget;

class PreferencesDialogUI : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialogUI(QWidget *parent = nullptr, QSettings *settings = nullptr);

private:
    QTreeWidget *navigationTree;
    QStackedWidget *stackedWidget;
    QSettings *settings;

    void setupUI();
    void setupNavigation();
};

#endif // PREFERENCESDIALOGUI_H
