#ifndef TASKSURFACE_H
#define TASKSURFACE_H

#include <Framework/robotbehaviour.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_PingerFollowing;
class Behaviour_XsensFollowing;

#define TASK_STATE_START                    "task started"
#define TASK_STATE_MOVE_TO_TASK_START       "move to task start"
#define TASK_STATE_PINGERFOLLOWING          "pingerfollowing"
#define TASK_STATE_END                      "   "
#define TASK_STATE_FIND_PING                "  "
#define TASK_STATE_END_FAILED               " "
class TaskSurface : public RobotBehaviour
{
    Q_OBJECT
public:
    // Methods
    TaskSurface(QString id, Module_Simulation *sim, Module_Navigation *n, Behaviour_PingerFollowing *pf, Behaviour_XsensFollowing *xf);
    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();
    bool isActive();
    QString getTaskState();

private:
    void init();
    bool running;
    void terminate();
    void reset();
    void showTaskState();

    // Controls all task states
    void controlTaskStates();

    void initBehaviourParameters();

    // Parameters
    bool active;
    Module_Simulation *sim;
    Module_Navigation *navi;
    Behaviour_XsensFollowing *xsensfollow;
    Behaviour_PingerFollowing *pingerfollow;

    QTimer taskTimer;

    QString taskState;

signals:
    // Update GUI input
    void updateSettings();

    // Show current state at command center
    void newState(QString task, QString state);
    // Overview of all task states at command center
    void newStateOverview(QString state);

public slots:
    // Task standard slots
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();
    void controlEnabledChanged(bool b);

    // Task specific state transitions
    void controlFinishedWaypoints(QString waypoint);
    void controlPingerState(QString newState);
    void controlXsensState(QString newState);

private slots:

};

#endif // TASKSURFACE_H
