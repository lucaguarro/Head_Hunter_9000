#include "llmconfigpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QDebug>

LLMConfigPage::LLMConfigPage(QWidget *parent, QSettings *settings)
    : QWidget(parent), m_settings(settings)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // --- Endpoint Row ---
    layout->addWidget(new QLabel("Ollama Endpoint:"));
    QHBoxLayout *endpointRow = new QHBoxLayout();

    endpointEdit = new QLineEdit(this);
    endpointEdit->setPlaceholderText("e.g., http://127.0.0.1:11434");

    if (m_settings) {
        QString savedEndpoint = m_settings->value("LLM_SERVER/ollama_endpoint", "").toString();
        endpointEdit->setText(savedEndpoint);
    }

    saveConnectionButton = new QPushButton("Save Connection", this);
    endpointRow->addWidget(endpointEdit);
    endpointRow->addWidget(saveConnectionButton);
    layout->addLayout(endpointRow);

    connectionResultLabel = new QLabel(this);
    connectionResultLabel->setVisible(false);
    layout->addWidget(connectionResultLabel);

    resultLabelTimer = new QTimer(this);
    resultLabelTimer->setSingleShot(true);
    connect(resultLabelTimer, &QTimer::timeout, [this]() {
        connectionResultLabel->setVisible(false);
    });

    connect(saveConnectionButton, &QPushButton::clicked, this, &LLMConfigPage::saveConnection);

    // --- Models Section ---
    layout->addWidget(new QLabel("Models Found:"));
    modelList = new QListWidget(this);
    layout->addWidget(modelList);

    // --- Manual Model Input ---
    layout->addWidget(new QLabel("Or type a model to add:"));
    modelNameEdit = new QLineEdit(this);
    modelNameEdit->setPlaceholderText("e.g., llama3, mistral, gemma");
    layout->addWidget(modelNameEdit);

    // --- Add Model Button ---
    addModelButton = new QPushButton("Add Model", this);
    layout->addWidget(addModelButton);

    layout->addWidget(new QLabel("Currently Selected Model:"));
    selectedModelBox = new QComboBox(this);
    layout->addWidget(selectedModelBox);

    // Save selected model when user changes selection
    connect(selectedModelBox, &QComboBox::currentTextChanged, this, [=](const QString &text) {
        if (m_settings && !text.isEmpty()) {
            m_settings->setValue("LLM_SERVER/selected_model", text);
            m_settings->sync();
            qDebug() << "Saved selected model:" << text;
        }
    });

    // --- Progress Bar ---
    downloadProgressBar = new QProgressBar(this);
    downloadProgressBar->setRange(0, 100); // percentage
    downloadProgressBar->setVisible(false);
    layout->addWidget(downloadProgressBar);

    connect(addModelButton, &QPushButton::clicked, this, &LLMConfigPage::pullSelectedModel);

    if (m_settings && !endpointEdit->text().isEmpty()) {
        QTimer::singleShot(0, this, &LLMConfigPage::saveConnection);
    }
}

void LLMConfigPage::saveConnection()
{
    QString endpoint = endpointEdit->text().trimmed();
    if (!endpoint.startsWith("http"))
        endpoint = "http://" + endpoint;

    disableButtons(true);

    QUrl url(endpoint + "/api/tags");
    QNetworkRequest request(url);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        disableButtons(false);

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray models = rootObj["models"].toArray();

            modelList->clear();

            // Block signals while repopulating to avoid unwanted "currentTextChanged"
            bool oldState = selectedModelBox->blockSignals(true);
            selectedModelBox->clear();

            for (const QJsonValue &modelVal : models) {
                QJsonObject modelObj = modelVal.toObject();
                QString modelName = modelObj["name"].toString();
                modelList->addItem(modelName);
                selectedModelBox->addItem(modelName);
            }

            // Restore selected model from settings, if available
            if (m_settings) {
                QString selectedModel = m_settings->value("LLM_SERVER/selected_model", "").toString();
                if (!selectedModel.isEmpty()) {
                    int index = selectedModelBox->findText(selectedModel);
                    if (index != -1) {
                        selectedModelBox->setCurrentIndex(index);
                    }
                }
            }

            selectedModelBox->blockSignals(oldState); // Restore previous block state

            if (m_settings) {
                m_settings->setValue("LLM_SERVER/ollama_endpoint", endpoint);
                m_settings->sync();
            }

            connectionResultLabel->setText("✅ Connected");
            connectionResultLabel->setStyleSheet("color: green;");
        } else {
            qDebug() << reply->error();
            connectionResultLabel->setText("❌ Failed to connect");
            connectionResultLabel->setStyleSheet("color: red;");
        }

        connectionResultLabel->setVisible(true);
        resultLabelTimer->start(3000);

        reply->deleteLater();
        manager->deleteLater();
    });
}

void LLMConfigPage::pullSelectedModel()
{
    QString endpoint = endpointEdit->text().trimmed();
    if (!endpoint.startsWith("http"))
        endpoint = "http://" + endpoint;

    QString modelName = modelNameEdit->text().trimmed();  // First check typed input
    if (modelName.isEmpty()) {
        modelName = selectedModelBox->currentText();       // Fallback to combo box
    }

    if (modelName.isEmpty()) {
        qDebug() << "No model specified.";
        return;
    }

    disableButtons(true);
    downloadProgressBar->setValue(0);
    downloadProgressBar->setVisible(true);

    QUrl url(endpoint + "/api/pull");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["model"] = modelName;
    json["stream"] = true; // we want streaming status updates

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::readyRead, this, [=]() {
        QByteArray chunk = reply->readAll();
        QList<QByteArray> lines = chunk.split('\n');
        for (const QByteArray &line : lines) {
            if (line.trimmed().isEmpty())
                continue;
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qDebug() << "JSON parse error:" << parseError.errorString();
                continue;
            }
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                QString status = obj["status"].toString();
                qDebug() << "Status:" << status;

                if (status == "success") {
                    downloadProgressBar->setValue(100);
                    connectionResultLabel->setText("✅ Model pulled successfully");
                    connectionResultLabel->setStyleSheet("color: green;");
                    connectionResultLabel->setVisible(true);
                    resultLabelTimer->start(3000);

                    saveConnection(); // Refresh model list after success
                } else if (obj.contains("total") && obj.contains("completed")) {
                    double total = obj["total"].toDouble();
                    double completed = obj["completed"].toDouble();
                    if (total > 0) {
                        int percent = static_cast<int>((completed / total) * 100.0);
                        downloadProgressBar->setValue(percent);
                    }
                }
            }
        }
    });

    connect(reply, &QNetworkReply::finished, this, [=]() {
        disableButtons(false);
        downloadProgressBar->setVisible(false);

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error pulling model:" << reply->errorString();
            connectionResultLabel->setText("❌ Failed to pull model");
            connectionResultLabel->setStyleSheet("color: red;");
            connectionResultLabel->setVisible(true);
            resultLabelTimer->start(3000);
        }

        reply->deleteLater();
        manager->deleteLater();
    });
}

void LLMConfigPage::disableButtons(bool disable)
{
    saveConnectionButton->setDisabled(disable);
    addModelButton->setDisabled(disable);
}
