#include "templatepage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

TemplatePage::TemplatePage(const QString &pageTitle,
                           const QString &placeholderText,
                           const QString &testerButtonText,
                           QWidget *parent)
    : QWidget(parent) {
    setupUI(pageTitle, placeholderText, testerButtonText);
}

void TemplatePage::setupUI(const QString &pageTitle,
                           const QString &placeholderText,
                           const QString &testerButtonText) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    instructionsLabel = new QLabel(
        QString("Write your prompt for the LLM to generate your %1 in the text area below.\n"
                "You can use the keywords:\n"
                "  • [Job Description]\n"
                "  • [Job Title]\n"
                "  • [Company]\n"
                "to give the LLM context of the specific job.")
            .arg(pageTitle.toLower()), this);
    instructionsLabel->setWordWrap(true);
    layout->addWidget(instructionsLabel);

    QHBoxLayout *selectLayout = new QHBoxLayout();
    QLabel *selectLabel = new QLabel("Select Template to Edit:", this);
    templateSelector = new QComboBox(this);
    templateSelector->addItem("None Selected");
    templateSelector->addItem("Example Template");
    selectLayout->addWidget(selectLabel);
    selectLayout->addWidget(templateSelector);
    layout->addLayout(selectLayout);

    templateEditor = new QTextEdit(this);
    templateEditor->setPlaceholderText(placeholderText);
    layout->addWidget(templateEditor);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    templateNameInput = new QLineEdit(this);
    templateNameInput->setPlaceholderText("Template Name");
    saveButton = new QPushButton("SAVE", this);
    deleteButton = new QPushButton("DELETE", this);
    actionLayout->addWidget(templateNameInput);
    actionLayout->addWidget(saveButton);
    actionLayout->addWidget(deleteButton);
    layout->addLayout(actionLayout);

    testerButton = new QPushButton(testerButtonText, this);
    layout->addWidget(testerButton);
}
