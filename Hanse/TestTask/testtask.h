#ifndef TESTTASK_H
#define TESTTASK_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>

class Module_Simulation;

class TestTask : public RobotBehaviour
{
    Q_OBJECT
public:
    TestTask(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    bool echoTest;

    QTimer testTimer;



private:
    void init();
    Behaviour_WallFollowing *wall;
    Module_Simulation *sim;
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

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
};

#endif // TESTTASK_H
