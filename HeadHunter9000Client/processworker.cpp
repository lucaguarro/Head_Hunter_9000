#include "processworker.h"

ProcessWorker::ProcessWorker(const QString& executable, const QStringList& arguments, QObject* parent)
    : QObject(parent), executable(executable), arguments(arguments), process(nullptr) {}

ProcessWorker::~ProcessWorker() {
    if (process) {
        process->deleteLater();
    }
}

void ProcessWorker::execute() {
    process = new QProcess(this);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessWorker::onProcessFinished);
    connect(process, &QProcess::errorOccurred,
            this, &ProcessWorker::onProcessError);

    // Start the process asynchronously
    process->start(executable, arguments);

    if (!process->waitForStarted()) {
        emit processError("Failed to start the process.");
    }
}

void ProcessWorker::stop() {
    if (process) {
        process->terminate(); // Send SIGTERM for graceful termination
        if (!process->waitForFinished(5000)) { // Wait 5 seconds
            process->kill(); // Force kill if not terminated
        }
    }
}

void ProcessWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QByteArray output = process->readAllStandardOutput();
    QByteArray errorOutput = process->readAllStandardError();
    emit processFinished(QString::fromLocal8Bit(output), QString::fromLocal8Bit(errorOutput));
}

void ProcessWorker::onProcessError(QProcess::ProcessError error) {
    QString errorMessage = process->errorString();
    emit processError(errorMessage);
}
