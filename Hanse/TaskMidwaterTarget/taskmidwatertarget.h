#ifndef TASKMIDWATERTARGET_H
#define TASKMIDWATERTARGET_H

#include <Framework/robotbehaviour.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_BallFollowing;
class Behaviour_XsensFollowing;

#define TASK_STATE_START                    "task started"
#define TASK_STATE_MOVE_TO_TASK_START       "move to task start"
#define TASK_STATE_FIND_BALL                "find ball"
#define TASK_STATE_AVOID_BALL               "avoid ball"
#define TASK_STATE_BALLFOLLOWING            "ballfollowing"
#define TASK_STATE_XSENSFOLLOW              "xsensfollowing"
#define TASK_STATE_MOVE_INSPECT             "move to inspect midwater target"
#define TASK_STATE_INSPECT                  "inspect midwater target"
#define TASK_STATE_END                      "task finished"
#define TASK_STATE_END_FAILED               "task finished - unsuccessful"

class TaskMidwaterTarget : public RobotBehaviour
{
    Q_OBJECT
public:
    // Methods
    TaskMidwaterTarget(QString id, Module_Simulation *sim, Module_Navigation *n, Behaviour_BallFollowing *bf, Behaviour_XsensFollowing *xf);
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
    Behaviour_BallFollowing *ballfollow;

    QTimer taskTimer;
    QTimer searchBallTimer;
    QString taskState;
    int tries;

signals:
    // Update GUI input
    void updateSettings();

    // Show current state at command center
    void newState(QString state);
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
    void controlBallState(QString newState);
    void controlXsensState(QString newState);

private slots:
    void markBallPosition();
    void cutFinished();
    void ballNotFound();
};

#endif // TASKMIDWATERTARGET_H
