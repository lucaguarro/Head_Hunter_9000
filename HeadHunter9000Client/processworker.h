#ifndef PROCESSWORKER_H
#define PROCESSWORKER_H

#include <QObject>
#include <QProcess>

class ProcessWorker : public QObject {
    Q_OBJECT
public:
    explicit ProcessWorker(const QString& executable, const QStringList& arguments, QObject* parent = nullptr);

public slots:
    void execute();

signals:
    void processFinished(const QString& output, const QString& errorOutput);
    void processError(const QString& errorMessage);

private:
    QString executable;
    QStringList arguments;
};

#endif // PROCESSWORKER_H
