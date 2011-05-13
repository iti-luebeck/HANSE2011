#ifndef TASKHANDCONTROL_H
#define TASKHANDCONTROL_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>


class Module_Simulation;

class TaskHandControl : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskHandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim, Module_HandControl *hc);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    bool echoTest;

    QTimer testTimer;



private:
    void init();
    Module_ThrusterControlLoop *thrustercontrolloop;
    Module_Simulation *sim;
    Module_HandControl *handcontrol;

    bool running;
    void terminate();

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void end();

    void getUiSettings();
    void newMessage(QString s);

    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);
    void setDepth(float depth);
    void handControlFinished();

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void handControlFinishedSlot();


};

#endif // TASKHANDCONTROL_H
