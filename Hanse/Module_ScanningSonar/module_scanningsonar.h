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
class Module_Simulation;

class Module_ScanningSonar : public RobotModule {
    Q_OBJECT

    friend class SonarDataSourceFile;
    friend class SonarDataSourceSerial;

public:
    Module_ScanningSonar(QString id,Module_ThrusterControlLoop *tcl,Module_Simulation *sim);
    ~Module_ScanningSonar();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    void emitSonarSignal();

public slots:
    bool doNextScan();

public slots:
    void reset();
    void terminate();

private slots:
    void gotEnabledChanged(bool);
    void refreshSimData(SonarReturnData data);

signals:
    void newSonarData(const SonarReturnData data);
    void requestSonarSignal();

private:
    Module_Simulation *sim;
    QTimer timer;
    SonarDataSource* source;
    SonarDataRecorder* recorder;
    void init();
};

#endif // MODULE_SCANNINGSONAR_H
