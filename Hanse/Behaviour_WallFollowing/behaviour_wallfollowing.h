#ifndef WALLFOLLOWINGBEHAVIOUR_H
#define WALLFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_WallFollowing/wallfollowingform.h>
#include <Module_EchoSounder/module_echosounder.h>
#include <Framework/eventthread.h>

class WallFollowingForm;
class Module_ThrusterControlLoop;
class Module_Simulation;

class Behaviour_WallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_WallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_EchoSounder *echo, Module_Simulation *sim);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    bool isActive();


private:
     //QTimer timer;
     void init();
     QString wallCase;

     void controlWallFollow();


     // void timerSlotExecute();

     Module_ThrusterControlLoop * tcl;
     Module_EchoSounder *echo;
     Module_Simulation *sim;
     EventThread updateThread;

    // int timerTime;
     bool running;

     float avgDistance;
     float distanceInput;
     float angSpeed;
     float fwdSpeed;
     float corridorWidth;
     void timerSlotExecute();

public slots:
     void updateFromSettings();
     void startBehaviour();
     void stop();
     void reset();
     void terminate();
     void stopOnEchoError();

     // Module_EchoSounder -> Behaviour_WallFollowing
     void newWallBehaviourData(const EchoReturnData data, float avgDistance);

signals:
     void timerStart( int msec );
     void timerStop();
     void dataError();

     void forwardSpeed(float fwSpeed);
     void angularSpeed(float anSpeed);

     void startControlTimer();

     // Behaviour_WallFollowing -> WallFollowingForm
     void newWallUiData(const EchoReturnData data, float avgDistance);
     void updateWallCase(QString caseW);
};


#endif // WallFollowingBEHAVIOUR_H
