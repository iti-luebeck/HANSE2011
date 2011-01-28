#ifndef MODULE_ECHOSOUNDER_H
#define MODULE_ECHOSOUNDER_H

#include "Framework/robotmodule.h"
#include "echoreturndata.h"
#include <Module_Simulation/module_simulation.h>
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

    class ThreadedReader : public QThread{
    public:
        ThreadedReader(Module_EchoSounder* m, Module_Simulation* sim);

        void pleaseStop();
        void run(void);

    private:
        Module_EchoSounder* m;
        Module_Simulation* sim;
        QTextStream* fileStream;
        bool running;
    };
public:
    Module_EchoSounder(QString id, Module_Simulation *sim);
    ~Module_EchoSounder();

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

    void emitEchoSignal();

public slots:
    bool doNextScan();

public slots:
    void reset();
    void terminate();

private slots:
    void gotEnabledChanged(bool);
    void refreshSimData(EchoReturnData data);

signals:
    void newEchoData(const EchoReturnData data);
    void requestEchoSignal();

private:
    Module_Simulation *sim;
    ThreadedReader reader;
    QTimer timer;
    EchoDataSource* source;
    EchoDataRecorder* recorder;

    void init();
};

#endif // MODULE_ECHOSOUNDER_H
