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
    QTimer moveToStartTimer;
    QTimer moveToEndTimer;
    QTimer doWallFollowTimer;
    QTimer controlNextStateTimer;

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

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();


    void moveToStart();
    void moveToEnd();
    void doWallFollow();
    void controlNextState();

};

#endif // TASKWALLNAVIGATION_H
