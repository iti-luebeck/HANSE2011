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
#define TASK_STATE_MOVE_TO_GATEWAYPOINT2    "move to gatewaypoint 1"
#define TASK_STATE_END                      "task finished"

class TaskPipeFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskPipeFollowing(QString id, Behaviour_PipeFollowing *w, Module_Simulation *sim, Module_Navigation *n, Behaviour_TurnOneEighty *o180);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QString getTaskState();



private:

    Behaviour_PipeFollowing *pipe;
    Module_Simulation *sim;
    Module_Navigation *navi;
    Behaviour_TurnOneEighty *turn180;

    void init();
    bool running;
    void terminate();

    QTimer taskTimer;
    QTimer angleTimer;

    int counter;

    QString taskState;
    QString pipeState;


    bool flag_PF_Pipe_Seen_Start;
    bool flag_PF_Part_1_Finished;
    bool flag_GoalLine_1_reached;
    bool flag_PF_Pipe_Seen;
    bool flag_PF_Part_2_Finished;
    bool flag_GoalLine_2_reached;

    void reset();

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void stopSignal();
    void updateSettings();

    void newState(QString state);
    void newStateOverview(QString state);

    void setUpdatePixmapSignal(bool b);

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();

    void moveToTaskStart();
    void moveToPipeStart();
    void pipefollowPart1();
    void moveToGatewaypoint1();
    void moveToPipe();
    void pipefollowPart2();
    void moveToGatewaypoint2();

    void controlAngleCalculation();
    void controlFinishedWaypoints(QString waypoint);
    void controlPipeState(QString newState);

    void controlEnabledChanged(bool b);

};

#endif // TASKPIPEFOLLOWING_H
