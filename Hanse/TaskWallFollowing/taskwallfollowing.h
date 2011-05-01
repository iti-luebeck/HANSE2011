#ifndef TASKWALLFOLLOWING_H
#define TASKWALLFOLLOWING_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>

class Module_Simulation;

class TaskWallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskWallFollowing(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim);

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
    void terminate();

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void end();

    void getUiSettings();

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void setRunData(int);

};

#endif // TASKWALLFOLLOWING_H
