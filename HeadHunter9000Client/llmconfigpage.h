#ifndef LLMCFGPAGE_H
#define LLMCFGPAGE_H

#include <QWidget>

class QLineEdit;
class QListWidget;
class QComboBox;
class QPushButton;
class QLabel;
class QTimer;

class LLMConfigPage : public QWidget {
    Q_OBJECT

public:
    explicit LLMConfigPage(QWidget *parent = nullptr);

private slots:
    void testConnection();

private:
    QLineEdit *endpointEdit;
    QListWidget *modelList;
    QComboBox *selectedModelBox;
    QPushButton *addModelButton;
    QPushButton *testConnectionButton;
    QLabel *connectionResultLabel;
    QTimer *resultLabelTimer;
};

#endif // LLMCFGPAGE_H
