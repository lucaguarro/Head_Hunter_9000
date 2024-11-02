#include "processworker.h"

ProcessWorker::ProcessWorker(const QString& executable, const QStringList& arguments, QObject* parent)
    : QObject(parent), executable(executable), arguments(arguments) {}

void ProcessWorker::execute() {
    QProcess pythonProcess;

    // Start the process with the executable and arguments
    pythonProcess.start(executable, arguments);

    // Wait for the process to finish (you might want to handle this asynchronously for large outputs)
    if (pythonProcess.waitForFinished()) {
        QByteArray output = pythonProcess.readAllStandardOutput();
        QByteArray errorOutput = pythonProcess.readAllStandardError();
        emit processFinished(QString(output), QString(errorOutput));
    } else {
        emit processError("Failed to execute the process.");
    }
}
