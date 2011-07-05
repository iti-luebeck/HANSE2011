#ifndef TASKTIMERSUBMERGED_H
#define TASKTIMERSUBMERGED_H

#include <Framework/robotbehaviour.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>


class Module_Simulation;

class TaskTimerSubmerged : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskTimerSubmerged(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QTimer stopTimer;



private:
    void init();
    Module_ThrusterControlLoop *thrustercontrolloop;
    Module_Simulation *sim;

    bool active;
    void terminate();

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void end();

    void getUiSettings();
    void newMessage(QString s);

    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);
    void setDepth(float depth);
    void newState(QString task, QString state);
    void newStateOverview(QString state);

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void controlEnabledChanged(bool b);
};

#endif // TASKTIMERSUBMERGED_H
