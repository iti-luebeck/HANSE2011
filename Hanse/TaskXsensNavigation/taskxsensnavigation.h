#ifndef TASKXSENSNAVIGATION_H
#define TASKXSENSNAVIGATION_H

#include <Framework/robotbehaviour.h>
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>
#include <Module_Navigation/module_navigation.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_XsensFollowing;

class TaskXsensNavigation : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskXsensNavigation(QString id, Module_Simulation *sim, Behaviour_XsensFollowing *xf, Module_Navigation *n);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QTimer taskTimer;
    QTimer moveToStartTimer;
    QTimer moveToEndTimer;
    QTimer doXsensFollowTimer;
    QTimer controlNextStateTimer;

private:
    Module_Simulation *sim;
    Behaviour_XsensFollowing *xsensfollow;
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


    void moveToStartSignal();
    void moveToEndSignal();
    void doXsensFollowSignal();
    void controlNextStateSignal();


public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();


    void moveToStartSlot();
    void moveToEndSlot();
    void doXsensFollowSlot();
    void controlNextStateSlot();

};

#endif // TASKXSENSNAVIGATION_H
