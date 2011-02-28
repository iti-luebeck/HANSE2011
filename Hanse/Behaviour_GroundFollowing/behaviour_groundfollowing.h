#ifndef BEHAVIOUR_GROUNDFOLLOWING_H
#define BEHAVIOUR_GROUNDFOLLOWING_H

#include <Framework/robotbehaviour.h>

#include <Module_EchoSounder/module_echosounder.h>
#include <Framework/eventthread.h>
#include <Behaviour_GroundFollowing/groundfollowingform.h>

class Module_ThrusterControlLoop;
class GroundFollowingForm;
class Module_Simulation;

class Behaviour_GroundFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_GroundFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_EchoSounder* echo, Module_Simulation *sim);
    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    /** returns true if Behaviour is active
        return false if the Behaviour is not active
    */
    bool isActive();

private:
    void init();

    /** Update data on data panel */
    void updateData();

    Module_ThrusterControlLoop* tcl;
    Module_EchoSounder* echo;
    Module_Simulation *sim;
    QTimer timer;
    EventThread updateThread;

private slots:
    void timerSlot();

public slots:
    /** updates local variables */
    void updateFromSettings();
    void startBehaviour();
    void stop();
    void reset();
    void terminate();

signals:
    void timerStart( int msec );
    void timerStop();

    // hoch/runter anstelle von forward/angular
    //void forwardSpeed(float fwSpeed);
    //void angularSpeed(float anSpeed);
};

#endif // BEHAVIOUR_GROUNDFOLLOWING_H
