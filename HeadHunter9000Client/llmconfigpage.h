#ifndef LLMCONFIGPAGE_H
#define LLMCONFIGPAGE_H

#include <QWidget>

class QSettings;
class QLineEdit;
class QPushButton;
class QLabel;
class QListWidget;
class QComboBox;
class QTimer;
class QProgressBar;

class LLMConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit LLMConfigPage(QWidget *parent = nullptr, QSettings *settings = nullptr);

private slots:
    void saveConnection();
    void pullSelectedModel();
    void disableButtons(bool disable);

private:
    QSettings *m_settings;

    QLineEdit *endpointEdit;
    QPushButton *saveConnectionButton;
    QLabel *connectionResultLabel;
    QTimer *resultLabelTimer;

    QListWidget *modelList;
    QPushButton *addModelButton;
    QComboBox *selectedModelBox;
    QLineEdit *modelNameEdit;    // <<---- NEW! Manual model name input
    QProgressBar *downloadProgressBar;
};

#endif // LLMCONFIGPAGE_H
