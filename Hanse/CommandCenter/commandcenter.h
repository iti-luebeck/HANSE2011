#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <Framework/robotmodule.h>
#include <QtCore>
#include <Framework/eventthread.h>
#include <TestTask/testtask.h>
#include <TaskWallFollowing/taskwallfollowing.h>

class Module_Simulation;
class Module_ThrusterControlLoop;
class Module_HandControl;
class Module_PressureSensor;
class Behaviour_PipeFollowing;
class Behaviour_BallFollowing;
class Behaviour_TurnOneEighty;
class Behaviour_WallFollowing;

class CommandCenter : public RobotModule
{
    Q_OBJECT
public:
    CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, TestTask *tt, TaskWallFollowing *tw);

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

    bool isActive();

    QList<QString> schedule;
    QString lTask;
    int count;


private:
    void init();
    QTimer timer;

    CommandCenter* c;
    Module_Simulation* sim;
    Module_ThrusterControlLoop* tcl;
    Module_HandControl* handControl;
    Module_PressureSensor* pressure;

    EventThread updateThread;

    // Different tasks
    TestTask *testtask;
    TaskWallFollowing *taskwallfollowing;

    // Timer for a short pause between the different tasks
    QTimer controlTimer;


    bool running;

    void commandCenterControl();
    void submergedExecute();

public slots:
    void startCommandCenter();
    void stopCommandCenter();

    void reset();
    void terminate();

    void finishedControl(RobotBehaviour*, bool success);
    void timeout();

private slots:
    void doNextTask();

signals:
    void setDepth(float depth);
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);



    void currentTask(QString s);
    void newError(QString s);
    void newAborted(QString s);


    void stopAllTasks();
    void resetTCL();

    void taskTimeout();

    void newList(QString s);

    void cStop();

    // Start and stop signals for every tasks
    void startTestTask();
    void stopTestTask();

    void startTaskWallFollowing();
    void stopTaskWallFollowing();


};

#endif // COMMANDCENTER_H
