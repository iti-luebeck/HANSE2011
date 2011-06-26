#ifndef TASKPIPEFOLLOWING_H
#define TASKPIPEFOLLOWING_H

#include <Framework/robotbehaviour.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_PipeFollowing;
class Behaviour_TurnOneEighty;

#define TASK_STATE_START                    "task started"
#define TASK_STATE_MOVE_TO_TASK_START       "move to task start"
#define TASK_STATE_MOVE_TO_PIPE_INIT        "move to pipe initial"
#define TASK_STATE_PIPEFOLLOW_PART1         "pipefollowing part 1"
#define TASK_STATE_MOVE_TO_GATEWAYPOINT1    "move to gatewaypoint 1"
#define TASK_STATE_MOVE_TO_PIPE             "move back to pipe"
#define TASK_STATE_PIPEFOLLOW_PART2         "pipefollowing part 2"
#define TASK_STATE_MOVE_TO_GATEWAYPOINT2    "move to gatewaypoint 2"
#define TASK_STATE_END                      "task finished"
#define TASK_STATE_END_FAILED               "task finished - unsuccessful"

class TaskPipeFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    // Methods
    TaskPipeFollowing(QString id, Behaviour_PipeFollowing *w, Module_Simulation *sim, Module_Navigation *n, Behaviour_TurnOneEighty *o180);
    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();
    bool isActive();
    QString getTaskState();

private:
    void init();
    void terminate();
    void reset();
    void showTaskState();
    void initBehaviourParameters();

    // Controls all task states
    void controlTaskStates();

    // Parameters
    bool active;
    Behaviour_PipeFollowing *pipe;
    Module_Simulation *sim;
    Module_Navigation *navi;
    Behaviour_TurnOneEighty *turn180;

    QTimer taskTimer;
    QTimer calcTimer;
    QString taskState;


signals:
    // Update GUI input
    void updateSettings();

    // Show current state at command center
    void newState(QString state);
    // Overview of all task states at command center
    void newStateOverview(QString state);

    void setUpdatePixmapSignal(bool b);

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
    void controlPipeState(QString newState);

};

#endif // TASKPIPEFOLLOWING_H
