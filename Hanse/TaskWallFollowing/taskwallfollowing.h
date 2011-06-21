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
    TaskWallFollowing(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim, Module_Navigation *n, Behaviour_TurnOneEighty *o180);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QString getTaskState();



private:

    Behaviour_WallFollowing *wall;
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


    bool flag_WF_Wall_Seen_Start;
    bool flag_GoalLine_reached;
    bool flag_AdjustHeading;

    void reset();

signals:
    void stopSignal();
    void updateSettings();

    void newState(QString state);
    void newStateOverview(QString state);


public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();

    void moveToTaskStart();
    void wallfollowPart1();
    void adjustHeading();
    void wallfollowPart2();

    void controlAngleCalculation();
    void controlFinishedWaypoints(QString waypoint);

    void controlEnabledChanged(bool b);

};

#endif // TASKWALLFOLLOWING_H
