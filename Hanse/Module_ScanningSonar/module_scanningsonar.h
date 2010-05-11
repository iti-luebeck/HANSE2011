#ifndef MODULE_SCANNINGSONAR_H
#define MODULE_SCANNINGSONAR_H

#include "Framework/robotmodule.h"
#include "sonarreturndata.h"
#include <QtCore>

class QextSerialPort;
//class SonarReturnData;
class SonarDataSource;
class SonarDataRecorder;

class Module_ScanningSonar : public RobotModule {
    Q_OBJECT

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
    Module_ScanningSonar(QString id);
    ~Module_ScanningSonar();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();
    QList<SonarReturnData*> retData;

public slots:
    void doNextScan();

public slots:
    void reset();
    void terminate();

signals:
    void newSonarData(SonarReturnData data);

private:
    ThreadedReader reader;
    QTimer timer;
    SonarDataSource* source;
    SonarDataRecorder* recorder;

};

#endif // MODULE_SCANNINGSONAR_H
