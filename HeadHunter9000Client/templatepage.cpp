#include "templatepage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QFile>
#include <QProcess>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSettings>
#include <QDebug>

TemplatePage::TemplatePage(const QString &pageTitle,
                           const QString &placeholderText,
                           const QString &testerButtonText,
                           QSettings *settings,
                           QWidget *parent)
    : QWidget(parent), m_settings(settings)
{
    setupUI(pageTitle, placeholderText, testerButtonText);

    if (!tempDir.isValid()) {
        qDebug() << "Warning: Could not create temporary directory.";
    }
}

void TemplatePage::setupUI(const QString &pageTitle,
                           const QString &placeholderText,
                           const QString &testerButtonText)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Instructions
    instructionsLabel = new QLabel(
        QString("Write your prompt for the LLM to generate your %1 below.\n"
                "Use [Job Title], [Company], [Job Description] as placeholders.")
            .arg(pageTitle.toLower()), this);
    instructionsLabel->setWordWrap(true);
    layout->addWidget(instructionsLabel);

    // Template selector
    QHBoxLayout *selectLayout = new QHBoxLayout();
    QLabel *selectLabel = new QLabel("Select Template to Edit:", this);
    templateSelector = new QComboBox(this);
    templateSelector->addItem("None Selected");
    templateSelector->addItem("Example Template");
    selectLayout->addWidget(selectLabel);
    selectLayout->addWidget(templateSelector);
    layout->addLayout(selectLayout);

    // Template editor
    templateEditor = new QTextEdit(this);
    templateEditor->setPlaceholderText(placeholderText);
    layout->addWidget(templateEditor);

    // Save/Delete
    QHBoxLayout *saveLayout = new QHBoxLayout();
    templateNameInput = new QLineEdit(this);
    templateNameInput->setPlaceholderText("Template Name");
    saveButton = new QPushButton("Save", this);
    deleteButton = new QPushButton("Delete", this);
    saveLayout->addWidget(templateNameInput);
    saveLayout->addWidget(saveButton);
    saveLayout->addWidget(deleteButton);
    layout->addLayout(saveLayout);

    // Tester button
    testerButton = new QPushButton(testerButtonText, this);
    layout->addWidget(testerButton);

    // Status Label
    statusLabel = new QLabel("Idle", this);
    statusLabel->setVisible(false);
    layout->addWidget(statusLabel);

    // Compile log output
    compileLog = new QTextEdit(this);
    compileLog->setReadOnly(true);
    compileLog->setVisible(false);
    layout->addWidget(compileLog);

    // PDF Viewer
    // pdfViewer = new QWebEngineView(this);
    // pdfViewer->setVisible(false);
    // layout->addWidget(pdfViewer);

    connect(testerButton, &QPushButton::clicked, this, &TemplatePage::testTemplate);
}

void TemplatePage::updateStatus(const QString &message, bool success)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(success ? "color: green;" : "color: red;");
    statusLabel->setVisible(true);
}

void TemplatePage::testTemplate()
{
    QString prompt = templateEditor->toPlainText().trimmed();
    if (prompt.isEmpty()) {
        updateStatus("Prompt is empty.", false);
        return;
    }

    testerButton->setDisabled(true);
    // pdfViewer->setVisible(false);
    updateStatus("Contacting Ollama server...", true);

    if (!m_settings) {
        updateStatus("Settings unavailable.", false);
        testerButton->setDisabled(false);
        return;
    }

    QString endpoint = m_settings->value("LLM_SERVER/ollama_endpoint", "").toString().trimmed();
    if (endpoint.isEmpty()) {
        updateStatus("Ollama endpoint not set.", false);
        testerButton->setDisabled(false);
        return;
    }

    if (!endpoint.startsWith("http"))
        endpoint = "http://" + endpoint;
    endpoint += "/api/generate";

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request((QUrl(endpoint)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload; // "LLM_SERVER/selected_model"
    QString selected_model = m_settings->value("LLM_SERVER/selected_model", "").toString().trimmed();
    if (selected_model.isEmpty()) {
        updateStatus("No model is selected", false);
        testerButton->setDisabled(false);
        return;
    }
    else {
        payload["model"] = selected_model;
    }
    payload["prompt"] = prompt;
    payload["stream"] = false;

    QNetworkReply *reply = manager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            updateStatus("Failed to contact Ollama.", false);
            testerButton->setDisabled(false);
            reply->deleteLater();
            manager->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        QJsonObject obj = jsonDoc.object();
        QString latexCode = obj["response"].toString();

        if (latexCode.isEmpty()) {
            updateStatus("Received empty LaTeX response.", false);
            testerButton->setDisabled(false);
            reply->deleteLater();
            manager->deleteLater();
            return;
        }

        if (!tempDir.isValid()) {
            updateStatus("Temporary directory not available.", false);
            testerButton->setDisabled(false);
            reply->deleteLater();
            manager->deleteLater();
            return;
        }

        QString texPath = tempDir.path() + "/generated.tex";
        QString pdfPath = tempDir.path() + "/generated.pdf";

        QFile texFile(texPath);
        if (!texFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            updateStatus("Failed to write LaTeX file.", false);
            testerButton->setDisabled(false);
            reply->deleteLater();
            manager->deleteLater();
            return;
        }
        texFile.write(latexCode.toUtf8());
        texFile.close();

        updateStatus("Saved LaTeX. Compiling...", true);

        // Compile
        QProcess *process = new QProcess(this);
        process->setWorkingDirectory(tempDir.path());
        process->start("xelatex", QStringList() << "-interaction=nonstopmode" << "generated.tex");

        connect(process, &QProcess::finished, this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
            QString output = process->readAllStandardOutput();
            QString errors = process->readAllStandardError();

            compileLog->clear();

            if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                updateStatus("Compiled successfully!", true);
                compileLog->setVisible(false);

                QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath));

            } else {
                updateStatus("LaTeX compilation failed.", false);

                // Show log output
                compileLog->setPlainText(errors.isEmpty() ? output : errors);
                compileLog->setVisible(true);
            }

            testerButton->setDisabled(false);
            process->deleteLater();
        });

        reply->deleteLater();
        manager->deleteLater();
    });
}
