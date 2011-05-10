#ifndef TASKWALLNAVIGATION_H
#define TASKWALLNAVIGATION_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
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

    EventThread updateThread;

    void init();
    bool running;
    void terminate();
    void controlTask();
    double distanceToStart;
    double distanceToTarget;

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void end();

    void updateSettings();


public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();


};

#endif // TASKWALLNAVIGATION_H
