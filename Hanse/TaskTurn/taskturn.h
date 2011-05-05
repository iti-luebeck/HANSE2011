#ifndef TASKTURN_H
#define TASKTURN_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_Compass/module_compass.h>

#define TURN_DEFAULT_P          0.4
#define TURN_DEFAULT_HYSTERESIS 10

class Module_Simulation;
class Module_ThrusterControlLoop;
class TaskTurn;

class TaskTurn : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskTurn(QString id, Module_ThrusterControlLoop *tcl, Module_PressureSensor *ps, Module_Compass *co, Module_XsensMTi *xsens, Module_Simulation *sim);
    //controlLoop, pressure, compass, xsens, sim

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    bool echoTest;

    QTimer *controlTimer;

    float currentHeading;
    float initialHeading;
    float targetHeading;
    float diffHeading;

    int tolerance;

    float angSpeed;
    float fwdSpeed;
    //float desDepth;


private:
    void init();
    Module_ThrusterControlLoop *thrustercontrolloop;
    Module_PressureSensor *pressure;
    Module_Compass *compass;
    Module_XsensMTi *xsens;

    Module_Simulation *sim;
    EventThread updateThread;

    bool running;
    void terminate();




signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void end();

    void getUiSettings();

    void newSchDesSignal(QString taskName, QString newD);
    void setDescriptionSignal();

    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);
    void setDepth(float depth);

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void setRunData(int);

    void newSchDesSlot(QString taskName, QString newD);
    void setDescriptionSlot();
    void controlTurn();
};

#endif // TASKTURN_H
