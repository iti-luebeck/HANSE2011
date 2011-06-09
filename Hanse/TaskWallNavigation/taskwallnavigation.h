#ifndef TASKWALLNAVIGATION_H
#define TASKWALLNAVIGATION_H

#include <Framework/robotbehaviour.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>
#include <Module_Navigation/module_navigation.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_WallFollowing;

class TaskWallNavigation : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskWallNavigation(QString id, Module_Simulation *sim, Behaviour_WallFollowing *w, Module_Navigation *n);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QTimer taskTimer;

private:
    Module_Simulation *sim;
    Behaviour_WallFollowing *wall;
    Module_Navigation *navi;

    void init();
    bool running;
    void terminate();
    double distanceToStart;
    double distanceToTarget;

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void stopSignal();
    void updateSettings();
    void newState(QString state);
    void newStateOverview(QString state);

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();

    void moveToStart();
    void seReached(QString waypoint);
    void moveToEnd();
    void doWallFollow();
    void controlWallFollowRemainingDistance();
    void controlNextState();

    void controlEnabledChanged(bool b);
};

#endif // TASKWALLNAVIGATION_H
