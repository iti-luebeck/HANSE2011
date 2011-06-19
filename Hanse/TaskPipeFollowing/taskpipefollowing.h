#ifndef TASKPIPEFOLLOWING_H
#define TASKPIPEFOLLOWING_H

#include <Framework/robotbehaviour.h>

class Module_Simulation;
class Module_Navigation;
class Behaviour_PipeFollowing;
class Behaviour_TurnOneEighty;

class TaskPipeFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskPipeFollowing(QString id, Behaviour_PipeFollowing *w, Module_Simulation *sim, Module_Navigation *n, Behaviour_TurnOneEighty *o180);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QTimer taskTimer;
    double distanceToPipeEnd;
    int counter;

private:

    Behaviour_PipeFollowing *pipe;
    Module_Simulation *sim;
    Module_Navigation *navi;
    Behaviour_TurnOneEighty *turn180;


    void init();
    bool running;
    void terminate();

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

    void moveToStart();
    void seReached(QString waypoint);
    void moveToEnd();
    void doPipeFollow();
    void doTurn();

    void controlPipeFollowRemainingDistance();
    void controlNextState();

    void controlEnabledChanged(bool b);

};

#endif // TASKPIPEFOLLOWING_H
