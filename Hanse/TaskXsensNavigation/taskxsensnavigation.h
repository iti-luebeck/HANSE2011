#ifndef TASKXSENSNAVIGATION_H
#define TASKXSENSNAVIGATION_H

#include <Framework/robotbehaviour.h>
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_XsensFollowing;
class Behaviour_TurnOneEighty;

class TaskXsensNavigation : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskXsensNavigation(QString id, Module_Simulation *sim, Behaviour_XsensFollowing *xf, Module_Navigation *n, Behaviour_TurnOneEighty *o180);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QTimer taskTimer;
    QTimer moveToStartTimer;
    QTimer moveToBTimer;
    QTimer moveToEndTimer;
    QTimer doTurnTimer;
    QTimer doXsensFollowTimer;
    QTimer controlNextStateTimer;

private:
    Module_Simulation *sim;
    Behaviour_XsensFollowing *xsensfollow;
    Module_Navigation *navi;
    Behaviour_TurnOneEighty *turn180;

    void init();
    bool running;
    void terminate();
    double distanceToStart;
    double distanceToTarget;
    double distanceToB;


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
    void moveToB();
    void moveToEnd();
    void doXsensFollow();
    void finishXsensFollow();
    void controlNextState();
    void doTurn();

};

#endif // TASKXSENSNAVIGATION_H
