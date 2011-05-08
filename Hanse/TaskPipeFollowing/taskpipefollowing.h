#ifndef TASKPIPEFOLLOWING_H
#define TASKPIPEFOLLOWING_H

#include <Framework/robotbehaviour.h>
#include <Framework/eventthread.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>

class Module_Simulation;

class TaskPipeFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskPipeFollowing(QString id, Behaviour_PipeFollowing *w, Module_Simulation *sim);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    bool echoTest;

    QTimer testTimer;

    int tempTask;

private:
    void init();
    Behaviour_PipeFollowing *pipe;
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

    void newSchDesSignal(QString taskName, QString newD);
    void setDescriptionSignal();
    void setUpdatePixmapSignal(bool b);

    void setRunDataSignal(int);


public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void setRunData(int);

    void newSchDesSlot(QString taskName, QString newD);
    void setDescriptionSlot();
    void updateTaskSettingsSlot();

};

#endif // TASKPIPEFOLLOWING_H
