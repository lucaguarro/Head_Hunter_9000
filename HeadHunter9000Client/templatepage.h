#ifndef TEMPLATEPAGE_H
#define TEMPLATEPAGE_H

#include <QWidget>
#include <QTemporaryDir>

class QLabel;
class QComboBox;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QWebEngineView;
class QSettings;

class TemplatePage : public QWidget
{
    Q_OBJECT

public:
    explicit TemplatePage(const QString &pageTitle,
                          const QString &placeholderText,
                          const QString &testerButtonText,
                          QSettings *settings,
                          QWidget *parent = nullptr);

private:
    void setupUI(const QString &pageTitle, const QString &placeholderText, const QString &testerButtonText);
    void testTemplate();
    void updateStatus(const QString &message, bool success = true);

    QSettings *m_settings;
    QTemporaryDir tempDir;

    QLabel *instructionsLabel;
    QComboBox *templateSelector;
    QTextEdit *templateEditor;
    QLineEdit *templateNameInput;
    QPushButton *saveButton;
    QPushButton *deleteButton;
    QPushButton *testerButton;
    QLabel *statusLabel;
    QTextEdit *compileLog;
    QWebEngineView *pdfViewer;
};

#endif // TEMPLATEPAGE_H
