#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <Framework/robotmodule.h>
#include <Framework/dataloghelper.h>
#include <QtCore>

class Module_Simulation;
class Module_ThrusterControlLoop;
class Module_HandControl;
class Module_PressureSensor;
class Behaviour_PipeFollowing;
class Behaviour_BallFollowing;
class Behaviour_TurnOneEighty;
class Behaviour_WallFollowing;
class Behaviour_XsensFollowing;
//class Behaviour_GoalFollowing;
class TaskHandControl;
class TaskWallFollowing;
class TaskXsensNavigation;
class RobotBehaviour;
class Module_Navigation;
class TaskPipeFollowing;
class TaskMidwaterTarget;
class TaskTimerSubmerged;
class TaskGotoWaypoint;


class CommandCenter : public RobotModule
{
    Q_OBJECT
public:
    CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, Module_Navigation* n, Behaviour_PipeFollowing* pipe, Behaviour_BallFollowing* ball, Behaviour_TurnOneEighty* o80, Behaviour_WallFollowing* wall, Behaviour_XsensFollowing* xsens, TaskHandControl *thc, TaskWallFollowing *twf, TaskXsensNavigation *txn, TaskPipeFollowing *tpf, TaskMidwaterTarget *mwt, TaskTimerSubmerged *tts, TaskGotoWaypoint* tgw);

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

    bool isActive();

    QList<RobotBehaviour*> taskList;
    QList<QString> taskInputList;
    QList<QString> scheduleList;
    QList<QString> finishedList;
    QList<QString> abortedList;
    QString activeTask;

private:
    void init();
    QTimer timer;
    QTimer stopTimer;
    QTimer startTimer;

    CommandCenter* c;
    Module_Simulation* sim;
    Module_ThrusterControlLoop* tcl;
    Module_HandControl* handControl;
    Module_PressureSensor* pressure;
    Behaviour_PipeFollowing* pipe;
    Behaviour_TurnOneEighty* o80;
    Behaviour_WallFollowing* wall;
    Behaviour_BallFollowing* ball;
    Behaviour_XsensFollowing* xsens;
    Module_Navigation* navi;

    QList<RobotBehaviour*> behaviour;

    // Behaviour_WallFollowing* goal;

    TaskHandControl *taskhandcontrol;
    TaskWallFollowing *taskwallfollowing;
    TaskXsensNavigation *taskxsensnavigation;
    TaskPipeFollowing *taskpipefollowing;
    TaskMidwaterTarget *taskmidwatertarget;
    TaskTimerSubmerged *tasktimersubmerged;
    TaskGotoWaypoint *taskgotowaypoint;

    QTimer controlTimer;

    bool active;

    void commandCenterControl();
    void submergedExecute();

    QFile *sauceLog;
    int filecount;
    QTime startTime;

    int logTime;
    double xKoord;
    double yKoord;
    double zKoord;
    QString currentTask;
    QString comment;
    QTextStream* stream;


    void sauceLogger(QString task, QString state);
    void close();



public slots:
    void startCommandCenter();
    void timerStartCommandCenter();
    void stopCommandCenter();

    void emergencyStopCommandCenter();

    void startTaskHandControlCC();
    void handControlFinishedCC(RobotBehaviour* name, bool success);
    void controlTaskHandControl(bool b);

    void reset();
    void terminate();

    void finishedControl(RobotBehaviour* name, bool success);

    void setNewMessage(QString s);


private slots:
    void doNextTask();
    void addTask(QString listName, QString taskName);
    void removeTask();
    void clearList(QString listName);
    void updateState(QString task, QString state);
    void updateStateOverview(QString state);
    void skipTask();
    void logPosition();

signals:
    void setDepth(float depth);
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);

    void newError(QString s);
    void newMessage(QString s);

    void newState(QString s);
    void newStateOverview(QString s);

    void stopAllTasks();
    void resetTCL();

    void cStop();

    void updateGUI();

    void eSB();

    // Start and stop signal for every tasks
    void startTaskHandControl();
    void stopTaskHandControl();

    void startTaskWallFollowing();
    void stopTaskWallFollowing();

    void startTaskXsensNavigation();
    void stopTaskXsensNavigation();

    void startTaskPipeFollowing();
    void stopTaskPipeFollowing();

    void startTaskMidwaterTarget();
    void stopTaskMidwaterTarget();

    void startTaskTimerSubmerged();
    void stopTaskTimerSubmerged();

    void startTaskGotoWaypoint();
    void stopTaskGotoWaypoint();
};

#endif // COMMANDCENTER_H
