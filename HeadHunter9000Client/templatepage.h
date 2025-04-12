#ifndef TEMPLATEPAGE_H
#define TEMPLATEPAGE_H

#include <QWidget>

class QLabel;
class QComboBox;
class QTextEdit;
class QLineEdit;
class QPushButton;

class TemplatePage : public QWidget {
    Q_OBJECT

public:
    explicit TemplatePage(const QString &pageTitle,
                          const QString &placeholderText,
                          const QString &testerButtonText,
                          QWidget *parent = nullptr);

protected:
    QLabel *instructionsLabel;
    QComboBox *templateSelector;
    QTextEdit *templateEditor;
    QLineEdit *templateNameInput;
    QPushButton *saveButton;
    QPushButton *deleteButton;
    QPushButton *testerButton;

    void setupUI(const QString &pageTitle,
                 const QString &placeholderText,
                 const QString &testerButtonText);
};

#endif // TEMPLATEPAGE_H
