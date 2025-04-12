#include "llmconfigpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

LLMConfigPage::LLMConfigPage(QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Ollama Endpoint Row + Test Button
    layout->addWidget(new QLabel("Ollama Endpoint:"));
    QHBoxLayout *endpointRow = new QHBoxLayout();
    endpointEdit = new QLineEdit(this);
    endpointEdit->setPlaceholderText("e.g., http://127.0.0.1:11434");
    testConnectionButton = new QPushButton("Test Connection", this);
    endpointRow->addWidget(endpointEdit);
    endpointRow->addWidget(testConnectionButton);
    layout->addLayout(endpointRow);

    // Result label (initially hidden)
    connectionResultLabel = new QLabel(this);
    connectionResultLabel->setVisible(false);
    layout->addWidget(connectionResultLabel);

    // Timer to hide result label
    resultLabelTimer = new QTimer(this);
    resultLabelTimer->setSingleShot(true);
    connect(resultLabelTimer, &QTimer::timeout, [=]() {
        connectionResultLabel->setVisible(false);
    });

    connect(testConnectionButton, &QPushButton::clicked, this, &LLMConfigPage::testConnection);

    // Models
    layout->addWidget(new QLabel("Models Found:"));
    modelList = new QListWidget(this);
    modelList->addItems({"llama-3.2", "llama-7.4", "llama-40.1"});
    layout->addWidget(modelList);

    addModelButton = new QPushButton("Add Model", this);
    layout->addWidget(addModelButton);

    layout->addWidget(new QLabel("Currently Selected Model:"));
    selectedModelBox = new QComboBox(this);
    selectedModelBox->addItems({"llama-3.2", "llama-7.4", "llama-40.1"});
    layout->addWidget(selectedModelBox);
}

void LLMConfigPage::testConnection() {
    QString endpoint = endpointEdit->text().trimmed();
    if (!endpoint.startsWith("http")) {
        endpoint = "http://" + endpoint;
    }

    QUrl url(endpoint + "/api/tags");
    QNetworkRequest request(url);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse JSON response
            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray models = rootObj["models"].toArray();

            // Clear and populate model list
            modelList->clear();
            selectedModelBox->clear();

            for (const QJsonValue &modelVal : models) {
                QJsonObject modelObj = modelVal.toObject();
                QString modelName = modelObj["name"].toString();
                modelList->addItem(modelName);
                selectedModelBox->addItem(modelName);
            }

            connectionResultLabel->setText("✅ Connected");
            connectionResultLabel->setStyleSheet("color: green;");
        } else {
            connectionResultLabel->setText("❌ Failed to connect");
            connectionResultLabel->setStyleSheet("color: red;");
        }

        connectionResultLabel->setVisible(true);
        resultLabelTimer->start(3000);

        reply->deleteLater();
        manager->deleteLater();
    });
}
