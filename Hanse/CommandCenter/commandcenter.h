#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <Framework/robotmodule.h>
#include <QtCore>
#include <Framework/eventthread.h>
#include <TaskHandControl/taskhandcontrol.h>
#include <TaskWallNavigation/taskwallnavigation.h>

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
    CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, TaskHandControl *thc, TaskWallNavigation *twn);

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

    bool isActive();

    QList<QString> scheduleList;
    QList<QString> finishedList;
    QList<QString> abortedList;
    QString activeTask;
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

    QTimer controlTimer;

    bool running;

    void commandCenterControl();
    void submergedExecute();

    TaskHandControl *taskhandcontrol;
    TaskWallNavigation *taskwallnavigation;

public slots:
    void startCommandCenter();
    void stopCommandCenter();

    void emergencyStopCommandCenter();
    void startTaskHandControlCC();

    void reset();
    void terminate();

    void finishedControl(RobotBehaviour*, bool success);
    void timeout();

    void setNewMessage(QString s);

    void handControlFinishedCC();

private slots:
    void doNextTask();


signals:
    void setDepth(float depth);
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);

    void newError(QString s);
    void newMessage(QString s);

    void stopAllTasks();
    void resetTCL();

    void taskTimeout();

    void cStop();

    void updateGUI();

    // Start and stop signal for every tasks
    void startTaskHandControl();
    void stopTaskHandControl();

    void startTaskWallNavigation();
    void stopTaskWallNavigation();


};

#endif // COMMANDCENTER_H
