#ifndef TESTTASK2_H
#define TESTTASK2_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>

class Module_Simulation;
class Module_ThrusterControlLoop;


class TestTask2 : public RobotBehaviour
{
    Q_OBJECT
public:
    TestTask2(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim, Module_ThrusterControlLoop* tcl);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    bool echoTest;

    QTimer testTimer;



private:
    void init();
    Behaviour_WallFollowing *wall;
    Module_Simulation *sim;
    Module_ThrusterControlLoop* tcl;
    EventThread updateThread;

    bool running;
    void reset();
    void terminate();
    void countdown();

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void end();

    void setDepth(float depth);

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
};

#endif // TESTTASK2_H
