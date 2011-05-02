#ifndef TASKTHRUSTERCONTROL_H
#define TASKTHRUSTERCONTROL_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>


class Module_Simulation;

class TaskThrusterControl : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskThrusterControl(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    bool echoTest;

    QTimer testTimer;



private:
    void init();
    Module_ThrusterControlLoop *thrustercontrolloop;
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
    void newMessage(QString s);

    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);
    void setDepth(float depth);

    void newSchDesSignal(QString taskName, QString newD);
    void setDescriptionSignal();

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void setRunData(int);

    void newSchDesSlot(QString taskName, QString newD);
    void setDescriptionSlot();

};

#endif // TASKTHRUSTERCONTROL_H
