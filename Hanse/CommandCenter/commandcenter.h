#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <Framework/robotmodule.h>
#include <QtCore>
#include <Framework/eventthread.h>
#include <TaskWallFollowing/taskwallfollowing.h>
#include <TaskThrusterControl/taskthrustercontrol.h>
#include <TaskPipeFollowing/taskpipefollowing.h>
#include <TaskTurn/taskturn.h>
#include <TaskHandControl/taskhandcontrol.h>

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
    CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, TaskWallFollowing *tw, TaskThrusterControl *ttc, TaskPipeFollowing *tp, TaskTurn *tt, TaskHandControl *thc);

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

    QTimer controlTimer;

    bool running;

    void commandCenterControl();
    void submergedExecute();

    TaskWallFollowing *taskwallfollowing;
    TaskThrusterControl *taskthrustercontrol;
    TaskPipeFollowing *taskpipefollowing;
    TaskTurn *taskturn;
    TaskHandControl *taskhandcontrol;

public slots:
    void startCommandCenter();
    void stopCommandCenter();

    void emergencyStopCommandCenter();

    void reset();
    void terminate();

    void finishedControl(RobotBehaviour*, bool success);
    void timeout();

    void setNewMessage(QString s);
    void newSchDesSlot(QString scheduleName, QString newD);

    void setDescriptionSlot();

    void handControlFinished();

private slots:
    void doNextTask();


signals:
    void setDepth(float depth);
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);



    void currentTask(QString s);
    void newError(QString s);
    void newAborted(QString s);
    void newMessage(QString s);



    void stopAllTasks();
    void resetTCL();

    void taskTimeout();

    void newList(QString s);

    void cStop();

    // Start and stop signals for every tasks
    void newSchDesSignal(QString scheduleName, QString newD);
    void setDescriptionSignal();

    void startTaskWallFollowing();
    void stopTaskWallFollowing();
    void setTaskWallFollowing(int taskNr);

    void startTaskThrusterControl();
    void stopTaskThrusterControl();
    void setTaskThrusterControl(int taskNr);


    void startTaskPipeFollowing();
    void stopTaskPipeFollowing();
    void setTaskPipeFollowing(int taskNr);

    void startTaskTurn();
    void stopTaskTurn();
    void setTaskTurn(int taskNr);

    void startTaskHandControl();
    void stopTaskHandControl();

};

#endif // COMMANDCENTER_H
