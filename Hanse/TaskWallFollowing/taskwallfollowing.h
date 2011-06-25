#ifndef TASKWALLFOLLOWING_H
#define TASKWALLFOLLOWING_H

#include <Framework/robotbehaviour.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_WallFollowing;
class Behaviour_TurnOneEighty;

#define TASK_STATE_START                    "task started"
#define TASK_STATE_MOVE_TO_TASK_START       "move to task start"
#define TASK_STATE_WALLFOLLOW_PART1         "wallfollowing part 1"
#define TASK_STATE_ADJUST_HEADING           "adjust heading"
#define TASK_STATE_WALLFOLLOW_PART2         "wallfollowing part 2"
#define TASK_STATE_END                      "task finished"
#define TASK_STATE_END_FAILED               "task finished - unsuccessful"

class TaskWallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    // Methods
    TaskWallFollowing(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim, Module_Navigation *n, Behaviour_TurnOneEighty *o180);
    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();
    bool isActive();
    QString getTaskState();

private:
    void init();
    void terminate();
    void reset();
    void showTaskState();

    // Controls all task states
    void controlTaskStates();

    // Parameters
    bool active;
    Behaviour_WallFollowing *wall;
    Module_Simulation *sim;
    Module_Navigation *navi;
    Behaviour_TurnOneEighty *turn180;

    QTimer taskStopTimer;
    QTimer calcTimer;
    QString taskState;

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
    void controlAngleDistance();
    void controlFinishedWaypoints(QString waypoint);

};

#endif // TASKWALLFOLLOWING_H
