#ifndef PROCESSWORKER_H
#define PROCESSWORKER_H

#include <QObject>
#include <QProcess>

class ProcessWorker : public QObject {
    Q_OBJECT
public:
    explicit ProcessWorker(const QString& executable, const QStringList& arguments, QObject* parent = nullptr);
    ~ProcessWorker();

public slots:
    void execute();
    void stop();

signals:
    void processFinished(const QString& output, const QString& errorOutput);
    void processError(const QString& errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    QString executable;
    QStringList arguments;
    QProcess* process;
};

#endif // PROCESSWORKER_H
