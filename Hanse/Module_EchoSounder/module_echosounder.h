#ifndef MODULE_ECHOSOUNDER_H
#define MODULE_ECHOSOUNDER_H

#include "Framework/robotmodule.h"
#include "echoreturndata.h"
#include <QtCore>

class QextSerialPort;
class EchoDataSource;
class EchoDataRecorder;
class Module_ThrusterControlLoop;
class Module_Simulation;

class Module_EchoSounder : public RobotModule{
    Q_OBJECT

    friend class EchoDataSourceFile;
    friend class EchoDataSourceSerial;

public:
    Module_EchoSounder(QString id, Module_Simulation *sim);

    const EchoReturnData d;

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

    float getAvgCm();
    float avgDistance;

protected:
    //void scanningOutput(const EchoReturnData data, int);
    int count;
    int range;
    int averageWindow;

    int threshold;
    float avgSig[251];
    float fewSigAvg[5][251];
    float calcFactor;

public slots:
    bool doNextScan();
    void scanningOutput(const EchoReturnData data);

public slots:
    void reset();
    void terminate();

    // Behaviour_WallFollowing -> Module_EchoSounder
    //void newEchoUiDataWE(float avgDistance, int averageWindow, int threshold, String port, int range);

private slots:
    void gotEnabledChanged(bool);
    void refreshSimData(EchoReturnData data);

signals:
    void newEchoData(const EchoReturnData data);

    //simulator stuff
    void requestSonarSideSignal();

    // Module_EchoSounder -> EchoSounderForm
    void newEchoUiData(float avgDistance, int averageWindow);

    // Module_EchoSounder -> Behaviour_WallFollowing (ggf. weiter)
    void newWallBehaviourData(const EchoReturnData data, float avgDistance);
   // void newWallUiDataEW(float avgDistance, int averageWindow, int threshold, String port, int range);

    void dataError();


private:
    Module_Simulation *sim;
    QTimer timer;
    EchoDataSource* source;
    EchoDataRecorder* recorder;

    void init();
};

#endif // MODULE_ECHOSOUNDER_H
