#ifndef PINGERFOLLOWINGBEHAVIOUR_H
#define PINGERFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>
#include <Module_Pinger/module_pinger.h>

class PingerFollowingForm;
class Module_ThrusterControlLoop;
class Module_Simulation;
class Module_XsensMTi;



class Behaviour_PingerFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    // Methods
    Behaviour_PingerFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_Pinger *pinger, Module_XsensMTi* x);
    QList<RobotModule*> getDependencies();
    QWidget* createView(QWidget *parent);
    bool isActive();


private:
    void init();
    void controlPingerFollow();


    // Parameters
    bool active;
    Module_Pinger *pinger;
    Module_XsensMTi* xsens;
    Module_ThrusterControlLoop * tcl;

signals:
    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);
    void updateUi();

public slots:
    void updateFromSettings();
    void startBehaviour();
    void stop();
    void reset();
    void terminate();
    void controlEnabledChanged(bool);

};


#endif
