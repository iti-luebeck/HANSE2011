#ifndef MODULE_SCANNINGSONAR_H
#define MODULE_SCANNINGSONAR_H

#include "Framework/robotmodule.h"
#include "sonarreturndata.h"
#include <QtCore>

class QextSerialPort;
//class SonarReturnData;
class SonarDataSource;
class SonarDataRecorder;
class Module_ThrusterControlLoop;

class Module_ScanningSonar : public RobotModule {
    Q_OBJECT

    friend class SonarDataSourceFile;
    friend class SonarDataSourceSerial;

    class ThreadedReader : public QThread {
    public:
        ThreadedReader(Module_ScanningSonar* m);

        void pleaseStop();

        void run(void);

    private:
        Module_ScanningSonar* m;
        QTextStream* fileStream;
        bool running; // XXX: volatile


    };

public:
    Module_ScanningSonar(QString id,Module_ThrusterControlLoop* tcl);
    ~Module_ScanningSonar();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    bool doNextScan();

public slots:
    void reset();
    void terminate();

private slots:
    void nextScanPeriod();

signals:
    void newSonarData(const SonarReturnData data);

private:
    Module_ThrusterControlLoop* tcl;
    ThreadedReader reader;
    QTimer timer;
    QTimer scanPeriodTimer;
    SonarDataSource* source;
    SonarDataRecorder* recorder;
    bool doScanning;
    int scanPeriodCntr;
};

#endif // MODULE_SCANNINGSONAR_H
