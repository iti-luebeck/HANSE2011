#ifndef TASKGOTOWAYPOINT_H
#define TASKGOTOWAYPOINT_H

#include <Framework/robotbehaviour.h>

class Module_Simulation;
class Module_Navigation;

#define GTW_STATE_START                    "task started"
#define GTW_STATE_MOVE_TO_WAYPOINT         "move to waypoint"
#define GTW_STATE_END                      "task finished"
#define GTW_STATE_END_FAILED               "task finished - unsuccessful"

class TaskGotoWaypoint : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskGotoWaypoint(QString id, Module_Simulation *sim, Module_Navigation *n);
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
    Module_Simulation *sim;
    Module_Navigation *navi;

    QTimer taskTimer;
    QString taskState;
    int waypointCounter;

signals:
    // Update GUI input
    void updateSettings();

    // Show current state at command center
    void newState(QString task,QString state);
    // Overview of all task states at command center
    void newStateOverview(QString state);

public slots:
    // Task standard slots
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();
    void controlEnabledChanged(bool b);
    void controlFinishedWaypoints(QString waypoint);
};

#endif // TASKGOTOWAYPOINT_H
