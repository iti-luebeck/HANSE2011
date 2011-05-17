#ifndef DATARECORDER_H
#define DATARECORDER_H

#include <QtCore>
#include <log4qt/logger.h>

class RobotModule;

class DataRecorder : public QObject
{
Q_OBJECT
public:
    DataRecorder(RobotModule& module);

    void stopRecording();

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

    void open();
    void close();
    bool isChanged(QStringList a, QStringList b);
};

#endif // DATARECORDER_H
