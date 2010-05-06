#ifndef DATARECORDER_H
#define DATARECORDER_H

#include <QtCore>
#include <robotmodule.h>
#include <log4qt/logger.h>

class DataRecorder : public QObject
{
Q_OBJECT
public:
    DataRecorder(RobotModule& module);

    void close();

signals:

public slots:
    void newDataReceived(RobotModule* module);

private:
    RobotModule& module;
    QFile *file;
    QTextStream* stream;
    Log4Qt::Logger *logger;

    QStringList settingsKeys;
    QStringList dataKeys;
    int fileCount;
    QString path;

    void open();
};

#endif // DATARECORDER_H
